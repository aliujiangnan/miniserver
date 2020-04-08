import time
import hashlib

def sign(o):
    d = hashlib.md5()
    d.update(o.encode(encoding='utf-8'))
    return d.hexdigest()

class Tokens:
    def __init__(self, id, time, period):
        self.id = id
        self.time = time
        self.period = period
		
class TokenMgr:
    @staticmethod
    def instance():
        return TokenMgr.gObj

    def __init__(self):
        self.allTokens = {}
        self.allTokensRef = {}
        self.allUsers = {}

    def newToken(self, id, period):
        token = self.getToken(id)
		if token != '' :
			delToken(token)

		now = time.time()
		tokenStr = sign(str(id) + '!@#$%^&' + str(now))
		self.allTokens[tokenStr] = Token(id, now, period)
		self.allTokensRef[tokenStr] = weakref.ref(self.allTokens[tokenStr])
		self.allUsers[id] = token
		return token

    def getToken(self, id):
        return self.allUsers.get(id)

    def getUserID(self, token):
        return self.allTokens.get(token).id


    def delToken(self, token):
        info = self.allTokens[token]
		if info != None :
            del self.allTokens[token]
			del self.allTokensRef[token]
			del self.allUsers[info.id]
        return

	def isValid(self, token):
        info = self.allTokens[token]
		if info != None :
			return False
		if info.time + info.period < time.time(): 
			return False
		return True

TokenMgr.gObj = TokenMgr()
