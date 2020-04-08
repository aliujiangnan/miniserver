# -*- coding: utf-8 -*-
import ffext
import math
import time
import json
import db
from common import MAX_NUM_EACH_TEAM
from player import Player
from player import PlayerMgr
from team import TeamMgr
from mapobj import MapObjMgr
# from tokens import TokenMgr

def init():
    print('py init.....')


def cleanup():
    print('py cleanup.....')

db.initDB()

# def test():
#     print('test timer.....')
        
# ffext.regTimer(5000, test)

def getLevel(exp):
    x = exp / 40
    if x < 1:
        return 1
    return int(math.log(x,2)) + 2

def gameBegin(player, team):
    players = []
    def putInfo(playerInTeam):
        info = {
            'id': playerInTeam.userID,
            'user': playerInTeam.userName,
            'nick': playerInTeam.userName,
            'online': playerInTeam.online,
            'gender': playerInTeam.gender,
            'avatar': playerInTeam.avatar,
            'score': playerInTeam.score,
            'level': playerInTeam.level,
        }
        players.append(info)
    team.foreach(putInfo)
    if team.hasRobot :
        info = {
            'id': player.userID + 1,
            'user': str(time.time() * 1000),
            'nick': str(time.time() * 1000),
            'online': True,
            'gender': 0,
            'avatar': 'http://sandbox-avatar.boochat.cn/2018/01/20/02/3/0042758573.gif',
            'score': 0,
            'level': 1,
        }
        players.append(info)
    data = {
        't':1,
        's':1,
        'id':team.teamID,
        'players':players,
        'robot':1 if team.hasRobot else 0,
    }
    player.broadcastInTeam('game.begin', data)


gMsgCallBack = {}
def bind(name):
    def funcWrap(func):
        global gMsgCallBack
        gMsgCallBack[name] = func
        return func
    return funcWrap
gIndexPlayerID = 0


@bind('login')
def login(player, data):
    userID = data['id']
    player.userName = data['user']
    player.nickName = data['nick']
    player.avatar = data['avatar']
    player.gender = data['gender']
    print('userid', userID)
    if userID == None:
        userID = db.createUser(player.userName,player.nickName,player.gender,player.avatar)
    if userID == None:
        return
    player.userID = userID

    global gIndexPlayerID
    gIndexPlayerID += 1
    PlayerMgr.instance().addPlayer(player)
    player.mapObj = MapObjMgr.instance().allocMap()
    player.mapObj.addPlayer(player)
    player.playerID = player.mapObj.allocPlayerID()
    player.score = db.getScore(userID, data['game'])
    player.score = 0 if player.score == None else player.score
    player.exp = db.getExp(userID, data['game'])
    print('exp===', player.exp)
    player.exp = 0 if player.exp == None else player.exp
    player.level = getLevel(player.exp)
    token = TokenMgr.newToken(userID, 7*24*3600*1000)
    player.sendMsg('game.init', {'token':token,'t': time.time(), 'id': userID,
                            'score': player.score, 'exp': player.exp, 'lv': player.level})

    return


@bind('match')
def handleMatch(player, data):
    team = player.mapObj.matchTeam(player, data['type'])
    print('teamid', team.teamID, 'num', team.getMemberNum())
    if team.getMemberNum() == MAX_NUM_EACH_TEAM:
        gameBegin(player, team)

    def putRobot():
        if team.getMemberNum() == MAX_NUM_EACH_TEAM or team.hasRobot:
            return
        team.hasRobot = True
        gameBegin(player, team)
        
    ffext.regTimer(15000, putRobot)
    return


@bind('msg')
def handleMsg(player, data):
    player.notifyOtherInTeam('player.msg', data)
    return


@bind('bcast')
def handleBroadcast(player, data):
    player.broadcastInTeam('player.msg', data)
    return


@bind('emoji')
def handleMove(player, data):
    player.notifyOtherInTeam('player.emoji', data)
    return


@bind('again')
def handleAgain(player, data):
    player.notifyOtherInTeam('player.again', data)
    return

@bind('accept')
def handleAccept(player, data):
    gameBegin(player, player.team)
    return


@bind('dissolve')
def handleDissolve(player, data):
    player.broadcastInTeam('game.dissolve', '')
    TeamMgr.instance().delTeam(player.team)
    player.mapObj.delTeam(player.team)
    player.team = None
    return


@bind('gameover')
def handleGameOver(player, data):
    rst = data['rst']
    def pushResult(playerInTeam):
        result = 0
        if playerInTeam.userID == player.userID:
            result = rst
        elif rst == 2:
            result = rst
        else:
            result = 1 if rst == 0 else 0
            
        expAdd = 20 if result == 0 else 40
        scoreAdd = -10 if result == 0 else 20
        beforeNext = int(40 * math.pow(2, playerInTeam.level)) - int(40 * math.pow(2, playerInTeam.level - 1))
        beforeLast = 0 if playerInTeam.level == 1 else int(40 * math.pow(2, playerInTeam.level - 1)) - int(40 * math.pow(2, player.level - 2))
        beforeExp = playerInTeam.exp - beforeLast
        curExp = playerInTeam.exp + expAdd
        curLv = getLevel(curExp)
        curScore = playerInTeam.score + scoreAdd
        curScore = 0 if curScore < 0 else curScore
        nextExp = int(40 * math.pow(2, curLv)) - int(40 * math.pow(2, curLv - 1))
        lastExp = 0 if curLv == 1 else int(40 * math.pow(2, curLv - 1)) - int(40 * math.pow(2, curLv - 2))

        data = {
            'curlv':curLv,
            'curscore':curScore,
            'befoexp':beforeExp,
            'befonext':beforeNext,
            'curexp':curExp - lastExp,
            'nextexp':nextExp,
            'scoreadd': ('+' if scoreAdd >= 0 else '') + str(scoreAdd),
            'expadd':'+' + str(expAdd),
        }

        player.score = curScore
        player.exp = curExp
        player.level = curLv
        db.updateScore(player.userID, player.team.type, player.score)
        db.addExp(player.userID, player.team.type, expAdd)
        playerInTeam.sendMsg('game.over', {'rst':result, 'data':data})

        if playerInTeam.online == False:
            PlayerMgr.instance().delPlayer(playerInTeam.userID)
            playerInTeam.team.delPlayer(playerInTeam.userID)
            mapObj = playerInTeam.mapObj
            mapObj.delPlayer(playerInTeam.userID)
            playerInTeam.mapObj = None

    player.team.foreach(pushResult)
    if player.team.getMemberNum < MAX_NUM_EACH_TEAM:
        TeamMgr.instance().delTeam(player.team.teamID)
        player.mapObj.delTeam(player.team.teamID)
        player.team = None

    return


@bind('ping')
def handlePing(player, data):
    player.sendMsg('game.pong','')
    return


def onSessionReq(sessionid, cmd, body):
    data = json.loads(body)
    name = data['n']
    if name not in ['target']:
        print('onSessionReq', sessionid, cmd, body)
    data = data['d']
    func = gMsgCallBack.get(name)
    if func:
        player = PlayerMgr.instance().getPlayerBySessonId(sessionid)
        if not player:
            if name == 'login':
                if checkMd5(data['sign'], data['id'], data['time']) == False and data['sign'] != 'notoken':
                    print('sign error')
                    return
                player = Player(sessionid)
            else:
                return
        func(player, data)
        #print('call %s.%s end'%(func.__module__, func.__name__))
        return
    
    #ip = ffext.getSessionIp(sessionid)
    #ffext.gateBroadcastMsg(cmd, '服务器收到消息，sessionid:%d,ip:%s,cmd:%d,data:%s'%(sessionid, ip, cmd, body))
    return


def onSessionOffline(sessionid):
    print('onSessionOffline', sessionid)
    player = PlayerMgr.instance().getPlayerBySessonId(sessionid)
    if not player:
        return

    if player.team:
        PlayerMgr.instance().delPlayer(player.userID)            
        mapObj = player.mapObj
        mapObj.delPlayer(player.userID)
        player.mapObj = None
    else:
        player.online = False
        player.notifyOtherInTeam('user.offline', {'id':player.userID})
        
    return



