
class Game:
    def __init__(self, id, n):
        self.gameID = id
        self.mapObj = None
        self.state = "idle"
        self.painterIndex = None
        self.wordIndex = None
        self.painterCount = 0
        self.selectTime = 0
        self.drawTime = 0
        self.showTime = 0
        self.endTime = 0
        self.playNum = 0
        self.words = []
        self.indics = []
        self.commands = []
        self.answers = []
        self.scores = [0]*n

class GameMgr:
    @staticmethod
    def instance():
        return GameMgr.gObj

    def __init__(self):
        self.allGames = {}

    def addGame(self, gameID, game):
        self.allGames[gameID] = game

    def getGame(self, gameID):
        return self.allGames.get(gameID)

    def delGame(self, gameID):
        if self.allGames.get(gameID):
            del self.allGames[gameID]
            return True
        return False

    def foreach(self, func):
        for k, game in self.allGames.iteritems():
            func(game)
        return

GameMgr.gObj = GameMgr()