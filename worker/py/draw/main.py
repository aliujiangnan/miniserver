# -*- coding: utf-8 -*-

import ffext
import math
import json
import time
import random
import db
from common import GAME_ID
from common import MAX_NUM_EACH_TEAM
from common import MAX_OB_EACH_TEAM
from player import Player
from player import PlayerMgr
from team import TeamMgr
from game import Game
from game import GameMgr
from mapobj import MapObjMgr
from tokens import TokenMgr

def init():
    print('py init.....')


def cleanup():
    print('py cleanup.....')

db.initDB()

def getLevel(exp):
    x = exp / 40
    if x < 1:
        return 1
    return int(math.log(x,2)) + 2

vocabularies = []
with open("script/draw/vocabularies.json", 'r') as loadF:
    vocabularies = json.load(loadF)


def getMembers(team):
    players = []
    obsers = []

    def putInfo(playerInTeam):
        info = {
            'id': playerInTeam.userID,
            'user': playerInTeam.userName,
            'nick': playerInTeam.userName,
            'online': playerInTeam.online,
            'ready': playerInTeam.ready,
            'gender': playerInTeam.gender,
            'avatar': playerInTeam.avatar,
            'score': playerInTeam.score,
            'exp': playerInTeam.exp,
            'isplayer': playerInTeam.isPlayer,
        }
        if playerInTeam.isPlayer:
            players.append(info)
        else:
            obsers.append(info)
    team.foreach(putInfo)

    return {'players': players, 'obsers': obsers}

def enterGame(player, team):
    info = None
    game = GameMgr.instance().getGame(team.teamID)
    if game == None:
        player.isPlayer = team.getPlayerNum() <= MAX_NUM_EACH_TEAM
    elif game.state == "selecting":
        info = {
            'state': "selecting",
            'idx': game.painterIndex,
            't': game.selectTime,
            'words': game.words,
        }
    elif game.state == "drawing":
        info = {
            'state': "drawing",
            'idx': game.painterIndex,
            't': game.drawTime,
            'word': game.words[game.wordIndex]['word'],
            'tip': game.words[game.wordIndex]['tip'],
            'cmds': game.commands,
        }
    elif game.state == "showing":
        info = {
            'state': "showing",
            'idx': game.painterIndex,
            't': game.showTime,
            'word': game.words[game.wordIndex]['word'],
            'cmds': game.commands,
        }
    elif game.state == "over":
        results = []
        info = {
            'state': "over",
            't': game.overTime,
            'scores': results,
        }

    members = getMembers(team)
    data = {
        's': 1,
        'id': player.team.teamID,
        'crter': player.team.creator,
        'isplayer': player.isPlayer,
        'members': members,
        'info': info,
    }
    player.sendMsg('game.enter', data)
    player.notifyOtherInTeam('user.new', {
                             'members': members, 'id': player.userID, 'isplayer': player.isPlayer})


def doGameOver(gameID):
    game = GameMgr.instance().getGame(gameID)
    if game == None:
        return

    team = TeamMgr.instance().getTeam(gameID)
    team.broadcast('game.over', {'rsts': game.scores})

    def saveAndReset(index, player):
        if player.online == False:
            PlayerMgr.instance().delPlayer(player.getID())
            player.team.delPlayer(player.userID)
            mapObj = player.mapObj
            mapObj.delPlayer(player.userID)
            player.mapObj = None
        else:
            player.ready = False

        player.score = game.scores[player.team.getPlayerIndex(player.userID)]
        add = 0 if player.score < 0 else player.score
        player.exp += add
        player.level = getLevel(player.exp)
        db.updateScore(player.userID, GAME_ID, player.score)
        db.addExp(player.userID, GAME_ID, add)

    team.foreachPlayer(saveAndReset)
    GameMgr.instance().delGame(gameID)
    game.mapObj.delGame(gameID)
    game.mapObj = None
    return


def genWords(gameID):
    game = GameMgr.instance().getGame(gameID)
    if game == None:
        return None

    words = []
    indics = []

    def gen():
        index = int(random.randint(0, 1000) / 1000.0 * len(vocabularies))
        for i in range(len(indics)):
            if index == indics[i]:
                return gen()
        for i in range(len(game.indics)):
            if index == game.indics[i]:
                return gen()

        v = vocabularies[index]
        indics.append(index)
        words.append({'tip': v['type'], 'word': v['word'], 'idx': index})

        if len(words) < 4:
            return gen()
        else:
            return words

    return gen()


def selectWord(gameID, quick):
    game = GameMgr.instance().getGame(gameID)
    if game == None:
        return

    team = TeamMgr.instance().getTeam(gameID)

    if game.painterCount < game.playNum:
        game.state = "selecting"
        game.selectTime = time.time()
        game.painterIndex = team.getPainterIndex(game.painterCount)
        game.painterCount += 1
        if game.painterIndex == None:
            return
    else:
        doGameOver(gameID)
        return

    def switch():
        game.selectTime = 0
        if game.wordIndex == None:
            selectWord(gameID, True)
        else:
            game.drawTime = time.time()
            data = {
                'idx': game.painterIndex,
                'time': game.drawTime,
                'word': game.words[game.wordIndex]['word'],
                'tip': game.words[game.wordIndex]['tip'],
            }
            game.state = "drawing"
            game.indics.append(game.words[game.wordIndex]['idx'])
            team.broadcast('game.draw', data)

            def show():
                showAnswer(gameID)
            ffext.regTimer(60000, show)
    ffext.regTimer(5000, switch)

    game.words = genWords(gameID)
    game.commands = []
    data = {
        'idx': game.painterIndex,
        'time': game.selectTime,
        'words': game.words,
        'quick': quick,
    }
    team.broadcast('game.select', data)


def showAnswer(gameID):
    game = GameMgr.instance().getGame(gameID)
    if game == None:
        return

    team = TeamMgr.instance().getTeam(gameID)
    game.state = "counting"
    game.showTime = time.time()

    point = num = 0
    for i in range(len(game.answers)):
        if game.answers[i]['answer'] == game.words[game.wordIndex]['word']:
            point += 2
            num += 2

    if num >= 5:
        point = 0

    data = {
        'num': num,
        'answer': game.words[game.wordIndex]['word'],
        'point': point,
        'score': game.scores[game.painterIndex]
    }

    team.broadcast('game.result', data)

    def bcast():
        game.state = "showing"
        team.broadcast('game.answer', '')
    ffext.regTimer(3000, bcast)

    def switch():
        game.showTime = 0
        game.answers = []
        game.wordIndex = None
        selectWord(gameID, False)
    ffext.regTimer(8000, switch)


def commitAnswer(player, answer):
    game = GameMgr.instance().getGame(player.team.teamID)
    if game == None or game.state != "drawing":
        return

    team = player.team
    index = team.getPlayerIndex(player.userID)
    if index == game.painterIndex:
        return

    for i in range(len(game.answers)):
        if answer == game.answers[i]['answer'] and index == i:
            return

    scores = [7, 5, 4, 3, 2]
    score = 0 if answer != game.words[game.wordIndex]['word'] else scores[len(
        game.answers)]
    game.answers.append({'userId': player.userID, 'answer': answer})
    game.scores[index] += score
    game.scores[game.painterIndex] += 2

    if len(game.answers) >= 3:
        num = 0
        for i in range(len(game.answers)):
            if game.answers[i]['answer'] == game.words[game.wordIndex]['word']:
                num += 1

        if num == 5:
            game.scores[game.painterIndex] -= 21

    data = {
        'id': player.userID,
        'answer': answer,
        'score': score,
    }
    player.broadcastInTeam('game.commit', data)


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
    player.score = db.getScore(userID, GAME_ID)
    player.score = 0 if player.score == None else player.score
    player.exp = db.getScore(userID, GAME_ID)
    player.exp = 0 if player.exp == None else player.exp
    player.level = getLevel(player.exp)
    token = TokenMgr.newToken(userID, 7*24*3600*1000)
    player.sendMsg('game.init', {'token':token,'t': time.time(), 'id': userID,
                            'score': player.score, 'exp': player.exp, 'lv': player.level})
    return


@bind('invite')
def handleInvite(player, data):
    team = player.team
    create = False
    if team == None:
        team = player.mapObj.allocTeam(player)
        team.creator = player.userID
        create = True
    player.broadcastInMap(
        'player.invite', {'n': player.nickName, 'id': player.userID, 'team': team.teamID})

    if create:
        enterGame(player, team)
    return


@bind('enter')
def handleEnter(player, data):
    team = TeamMgr.instance().getTeam(data['id'])
    game = GameMgr.instance().getGame(team.teamID)
    if player.team != None or team == None or team.getMemberNum() >= MAX_NUM_EACH_TEAM + MAX_OB_EACH_TEAM:
        return
    elif game != None and team.getMemberNum() < MAX_NUM_EACH_TEAM:
        player.team = team
        team.addObser(player)
    else:
        player.team = team
        team.addPlayer(player)

    enterGame(player, team)
    return


@bind('ready')
def handleReady(player, data):
    if player.ready:
        return

    player.ready = True
    player.broadcastInTeam('player.ready', {'id': player.userID})
    n = player.team.getOlPlayerNum()
    if n > 1 and player.team.getReadyNum() == n:
        player.broadcastInTeam('game.allready', '')

        def callback():
            game = Game(player.team.teamID, player.team.getPlayerNum())
            game.mapObj = player.mapObj
            GameMgr.instance().addGame(game.gameID, game)
            player.mapObj.addGame(game.gameID, game)
            game.painterIndex = 0
            player.broadcastInTeam('game.begin', {'idx': game.painterIndex})
            game.state = "begin"
            game.playNum = n
            selectWord(game.gameID, False)
            return
        ffext.regTimer(5000, callback)
    return


@bind('standup')
def handleStand(player, data):
    rst = player.team.standup(player)
    if rst:
        members = getMembers(player.team)
        player.broadcastInTeam(
            'user.standup', {'id': player.userID, 'members': members})
    return


@bind('sitdown')
def handleSit(player, data):
    rst = player.team.sitdown(player)
    if rst:
        members = getMembers(player.team)
        player.broadcastInTeam(
            'user.sitdown', {'id': player.userID, 'members': members})
    return


@bind('tool')
def handleTool(player, data):
    game = GameMgr.instance().getGame(player.team.teamID)
    if game == None:
        return
    game.commands.append(data)
    player.notifyOtherInTeam('cmd.tool', data)
    return

@bind('shape')
def handleDraw(player, data):
    game = GameMgr.instance().getGame(player.team.teamID)
    if game == None:
        return
    game.commands.append({'type': 'draw', 'data': data})
    player.notifyOtherInTeam('cmd.shape', data)
    return


@bind('select')
def handleSelect(player, data):
    game = GameMgr.instance().getGame(player.team.teamID)
    if game == None:
        return
    game.wordIndex = data['idx']
    return


@bind('chat')
def handleChat(player, data):
    player.notifyOtherInTeam('game.chat', data)
    return


@bind('commit')
def handleCommit(player, data):
    commitAnswer(player, data['str'])
    return


@bind('cancel')
def handleCancel(player, data):
    player.ready = True
    player.broadcastInTeam('player.cancel', {'id': player.userID})
    return


@bind('kick')
def handleKick(player, data):
    target = PlayerMgr.instance().getPlayer(data['id'])
    if target != None:
        player.team.delPlayer(data['id'])
        target.team = None
        player.team.broadcast('user.state', {'id': data['id'], 'online': False, 'members': getMembers(player.team)})
        target.sendMsg('game.kick', '')
    return

@bind('exit')
def handleExit(player, data):
    team = player.team
    userID = player.userID
    if team.creator == userID:
        return
    
    player.team.delPlayer(userID)
    player.team = None
    team.broadcast('user.state', {'id': userID,'online':False,'members':getMembers(team)})
    player.sendMsg('game.exit', '')
    return

@bind('dissolve')
def handleDissolve(player, data):
    if player.userID != player.team.creator:
        return

    player.broadcastInTeam('game.exit', '')

    teamID = player.team.teamID
    def clear(player):
        player.team = None
    player.team.foreach(clear)
    
    TeamMgr.instance().delTeam(teamID)
    player.mapObj.delTeam(teamID)
    return

@bind('ping')
def handlePing(player, data):
    player.sendMsg('game.pong', '')
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
        # print('call %s.%s end'%(func.__module__, func.__name__))
        return

    # ip = ffext.getSessionIp(sessionid)
    # ffext.gateBroadcastMsg(cmd, '服务器收到消息，sessionid:%d,ip:%s,cmd:%d,data:%s'%(sessionid, ip, cmd, body))
    return


def onSessionOffline(sessionid):
    print('onSessionOffline', sessionid)
    player = PlayerMgr.instance().getPlayerBySessonId(sessionid)
    if not player:
        return
   
    if player.team == None:
        PlayerMgr.instance().delPlayer(player.userID)            
        mapObj = player.mapObj
        mapObj.delPlayer(player.userID)
        player.mapObj = None
    else:
        game = GameMgr.instance().getGame(player.team.teamID)

        if game == None and player.userID != player.team.creator:
            team = player.team
            PlayerMgr.instance().delPlayer(player.userID)
            player.team.delPlayer(player.userID)
            player.team = None            
            mapObj = player.mapObj
            mapObj.delPlayer(player.userID)
            player.mapObj = None
            team.broadcast('user.state', {'id': player.userID,'online':False,'members':getMembers(team)})
        else:
            player.online = False
            player.team.broadcast('user.state', {'id': player.userID,'online':False,'members':getMembers(player.team)})
        
    return

