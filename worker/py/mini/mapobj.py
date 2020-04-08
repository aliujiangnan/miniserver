import weakref
from common import MAX_NUM_EACH_MAP
from common import MAX_NUM_EACH_TEAM
from team import Team
from team import TeamMgr

gID = 0
def allocID():
    global gID
    gID += 1
    return gID

class MapObj:
    def __init__(self, mapid):
        self.mapid  = mapid
        self.allPlayers = {}
        self.allTeams  = {}
    def getPlayerNum(self):
        return len(self.allPlayers)
    def addPlayer(self, player):
        self.allPlayers[player.userID] = weakref.ref(player)
    def getPlayer(self, userID):
        ret = self.allPlayers.get(userID)
        if ret != None:
            return ret()
        return None
    def delPlayer(self, userID):
        if self.allPlayers.get(userID):
            del self.allPlayers[userID]

            def delInTeam(team):
                team.delPlayer(userID)
                return
            
            TeamMgr.instance().foreach(delInTeam)
            return True
        return False
    def foreach(self, func):
        for k, playerref in self.allPlayers.iteritems():
            func(playerref())
        return True
    def allocPlayerID(self):
        allExistID = []
        for k, playerref in self.allPlayers.iteritems():
            player = playerref()
            allExistID.append(player.playerID)
        for k in range(1, 100):
            if k not in allExistID:
                return k
        return 1
    def getTeamNum(self):
        return len(self.allTeams)
    def addTeam(self, team):
        self.allTeams[team.teamID] = weakref.ref(team)
    def getTeam(self, teamID):
        ret = self.allPlayers.get(teamID)
        if ret != None:
            return ret()
        return None
    def delTeam(self, teamID):
        if self.allTeams.get(teamID):
            del self.allTeams[teamID]
            return True
        return False
    def matchTeam(self, player, teamType):
        retTeam = None
        for k, teamref in self.allTeams.iteritems():
            team = teamref()
            if team.getMemberNum() < MAX_NUM_EACH_TEAM and team.type == teamType and team.hasRobot == False:
                retTeam = team
        
        if retTeam == None:
            retTeam = Team(self.allocTeamID())
            retTeam.type = teamType

        player.team = retTeam
        retTeam.addPlayer(player)
        self.addTeam(retTeam)
        TeamMgr.instance().addTeam(retTeam)
        return retTeam
    def allocTeamID(self):
        allExistID = []
        for k, teamref in self.allTeams.iteritems():
            team = teamref()
            allExistID.append(team.teamID)
        for k in range(1, 100):
            if k not in allExistID:
                return k
        return 1

class MapObjMgr:
    gObj = None
    @staticmethod
    def instance():
        return MapObjMgr.gObj
    def __init__(self):
        self.allMaps = {}
        return
    def allocMap(self):
        retMap = None
        for k, mapObj in self.allMaps.iteritems():
            if mapObj.getPlayerNum() < MAX_NUM_EACH_MAP:
                retMap = mapObj
                break
        if not retMap:
            retMap= MapObj(allocID())
            self.allMaps[retMap.mapid] = retMap
        return retMap    
            
MapObjMgr.gObj = MapObjMgr()