import collections

class Team:
    def __init__(self, id):
        self.teamID= id
        self.type = 0
        self.hasRobot = False
        self.allPlayers = collections.OrderedDict()
    def addPlayer(self, player):
        self.allPlayers[player.userID] = player
    def getPlayer(self, userID):
        ret = self.allPlayers.get(userID)
        if ret != None:
            return ret()
        return None
    def delPlayer(self, userID):
        if self.allPlayers.get(userID):
            del self.allPlayers[userID]
            return True
        return False
    def getMemberNum(self):
        return len(self.allPlayers)
    def foreach(self, func):
        for k, player in self.allPlayers.iteritems():
            func(player)
        return

class TeamMgr:
    @staticmethod
    def instance():
        return TeamMgr.gObj
    def __init__(self):
        self.allTeams = {}
    def addTeam(self, team):
        self.allTeams[team.teamID] = team
    def getTeam(self, id):
        return self.allTeams.get(id)
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