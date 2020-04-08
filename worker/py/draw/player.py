import ffext
import json

def sendTo(sessionid, cmd, data=''):
    strData = json.dumps({'n': cmd, 'd': data})
    if cmd != 'update':
        print(strData)
    return sendRaw(sessionid, strData)

def sendRaw(sessionid, strData):
    return ffext.sessionSendMsg(sessionid, 0, strData)

class Player:
    def __init__(self, sessionid):
        self.sessionid = sessionid
        self.mapObj = None
        self.team = None
        self.playerID = 0
        self.userID = None
        self.userName = None
        self.nickName = None
        self.avatar= None
        self.gender = 0
        self.score = 0
        self.exp = 0
        self.ready = False
        self.online = True
        self.isPlayer = False

    def getID(self):
        return self.sessionid

    def sendMsg(self, name, data):
        return sendTo(self.sessionid, name, data)

    def broadcastInMap(self, name, data):
        if self.mapObj:
            self.mapObj.foreach(lambda player: player.sendMsg(name, data))
        return

    def broadcastInTeam(self, name, data):
        if self.team:
            self.team.foreach(lambda player: player.sendMsg(name, data))
        return

    def notifyOtherInTeam(self, name, data):
        if self.team:
            def sendData(player):
                if player.getID() != self.sessionid:
                    player.sendMsg(name, data)
                return
            self.team.foreach(sendData)
        return

class PlayerMgr:
    gObj = None
    @staticmethod
    def instance():
        return PlayerMgr.gObj

    def __init__(self):
        self.allPlayers = {}
        self.allSessions = {}
    def addPlayer(self, player):
        self.allPlayers[player.userID] = player
        self.allSessions[player.getID()] = player.userID

    def getPlayer(self, id):
        return self.allPlayers.get(id)

    def getPlayerBySessonId(self, id):
        return self.allPlayers.get(self.allSessions.get(id))
    def delPlayer(self, userID):
        if self.allPlayers.get(userID):
            sessionID = self.getPlayer(userID).getID()
            del self.allPlayers[userID]
            if self.allSessions.get(sessionID):
                del self.allSessions[sessionID]
            return True
        return False

    def foreach(self, func):
        for k, player in self.allPlayers.iteritems():
            func(player)
        return

PlayerMgr.gObj = PlayerMgr()