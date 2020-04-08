import collections
from common import MAX_NUM_EACH_TEAM
from common import MAX_OB_EACH_TEAM 

class Team:
    def __init__(self, id):
        self.teamID = id
        self.type = 0
        self.creator = None
        self.allPlayers = collections.OrderedDict()
        self.allObsers = collections.OrderedDict()

    def addPlayer(self, player):
        self.allPlayers[player.userID] = player

    def getPlayer(self, userID):
        ret = self.allPlayers.get(userID)
        if ret != None:
            return ret()
        return None

    def getPlayerIndex(self, userID):
        i = 0
        for k, player in self.allPlayers.iteritems():
            if player.userID == userID:
                break
            i += 1
        return i

    def getPlayerByIndex(self, index):
        n = 0
        for k, player in self.allPlayers.iteritems():
            if n == index:
                return player
            n += 1
        return None

    def delPlayer(self, userID):
        if self.allPlayers.get(userID):
            del self.allPlayers[userID]
            return True
        return False

    def addObser(self, player):
        self.allObsers[player.userID] = player

    def getObser(self, userID):
        ret = self.allPlayers.get(userID)
        if ret != None:
            return ret()
        return None

    def delObser(self, userID):
        if self.allObsers.get(userID):
            del self.allObsers[userID]
            return True
        return False

    def getMemberNum(self):
        return len(self.allPlayers) + len(self.allObsers)

    def getPlayerNum(self):
        return len(self.allPlayers)

    def getObserNum(self):
        return len(self.allObsers)

    def getReadyNum(self):
        i = 0
        for k, player in self.allPlayers.iteritems():
            if player.ready:
                i += 1
        return i

    def standup(self, player):
        if self.getObserNum() < MAX_OB_EACH_TEAM:
            self.delPlayer(player.userID)
            self.addObser(player)
            player.isPlayer = False
            player.ready = False
            return True
        else:
            return False

    def sitdown(self, player):
        if self.getPlayerNum() < MAX_NUM_EACH_TEAM:
            self.delObser(player.userID)
            self.addPlayer(player)
            player.isPlayer = True
            player.ready = False
            return True
        else:
            return False

    def foreach(self, func):
        for k, player in self.allPlayers.iteritems():
            func(player)
        for k, player in self.allObsers.iteritems():
            func(player)
        return

    def foreachPlayer(self, func):
        i = 0
        for k, player in self.allPlayers.iteritems():
            func(i, player)
            i += 1
        return

    def foreachObser(self, func):
        i = 0
        for k, player in self.allObsers.iteritems():
            func(i, player)
            i += 1
        return

    def broadcast(self, name, data):
        def bcast(player):
            player.sendMsg(name, data)
        self.foreach(bcast)
        return

    def getOlPlayerNum(self):
        n = 0
        for k, player in self.allPlayers.iteritems():
            if player.online:
                n += 1
        return n

    def getPainterIndex(self, count):
        index = None
        n = i = 0
        for k, player in self.allPlayers.iteritems():
            if player.online:
                if n == count:
                    index = i
                    break
                n += 1
            i += 1
        return index

class TeamMgr:
    @staticmethod
    def instance():
        return TeamMgr.gObj

    def __init__(self):
        self.allTeams = {}

    def addTeam(self, team):
        self.allTeams[team.teamID] = team

    def getTeam(self, teamID):
        return self.allTeams.get(teamID)

    def delTeam(self, teamID):
        if self.allTeams.get(teamID):
            del self.allTeams[teamID]
            return True
        return False

    def foreach(self, func):
        for k, team in self.allTeams.iteritems():
            func(team)
        return

TeamMgr.gObj = TeamMgr()