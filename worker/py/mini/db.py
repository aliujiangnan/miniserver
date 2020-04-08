# -*- coding: utf-8 -*-

import ffext

cfg = "sqlite://./game.db"

def query(sql):
    # print('sql', sql)
    ret = ffext.queryByCfg(cfg, sql)
    # print('ret', ret)
    if(ret['errinfo']):
        # print('dberror: ' + ret['errinfo'])
        return None
    elif len(ret['datas']) == 0:
        # print('dberror: no record')
        return None
    return ret

def initDB():
    ret = query('create table IF NOT EXISTS user (\
        id INTEGER PRIMARY  KEY     AUTOINCREMENT,\
        username            TEXT    NOT NULL,\
        nickname            TEXT,\
        avatar              CHAR(128),\
        gender              INT\
        );')

    ret = query('create table IF NOT EXISTS userinfo (\
        userid              INT     NOT NULL,\
        gameid              INT     NOT NULL,\
        score               INT     NOT NULL,\
        exp                 INT     NOT NULL\
        );')
    return ret != None

def createUser(username, nickname, gender, avatar):
    ret = query('INSERT INTO user (username,nickname,avatar,gender)\
        VALUES (\''+ username + '\', \'' + nickname + '\', ' + str(gender) + ', \'' + avatar + '\' );')
    return len(query('SELECT * FROM user')['datas'])

def getUser(userid):
    ret = query('SELECT * FROM user WHERE id = ' + str(userid) + ';')
    if ret:
        return ret['datas'][0][0]
    return ret

def getUserInfo(userid, gameid):
    ret = query('SELECT * FROM userinfo WHERE userid = ' + str(userid) + ' AND gameid = ' + str(gameid) + ';')
    if ret:
        return ret['datas'][0][0]
    return ret

def getScore(userid, gameid):
    ret = query('SELECT score FROM userinfo WHERE userid = ' + str(userid) + ' AND gameid = ' + str(gameid) + ';')
    if ret:
        return int(ret['datas'][0][0])
    return ret

def getExp(userid, gameid):
    ret = query('SELECT exp FROM userinfo WHERE userid = ' + str(userid) + ' AND gameid = ' + str(gameid) + ';')
    if ret:
        return int(ret['datas'][0][0])
    return ret

def updateScore(userid, gameid, score):
    ret = getScore(userid, gameid)
    if ret != None and ret > score:
        return False

    if getUserInfo(userid, gameid) == None:
        ret = query('INSERT INTO userinfo (userid,gameid,score,exp)\
            VALUES (' + str(userid) + ', ' + str(gameid) + ', ' + str(score) + ', 0);')
    else:
        ret = query('UPDATE userinfo SET score = ' + str(score) + ' WHERE userid = ' + str(userid) + ' AND gameid = ' + str(gameid) + ';')
    return ret != None

def addExp(userid, gameid, add):
    if add < 0:
        return False
    ret = None
    
    if getUserInfo(userid, gameid) == None:
        ret = query('INSERT INTO userinfo (userid,gameid,score,exp)\
            VALUES (' + str(userid) + ', ' + str(gameid) + ', 0, ' + str(add) + ');')
    else:
        cur = getExp(userid, gameid)
        ret = query('UPDATE userinfo SET exp = ' + str(cur + add) + ' WHERE userid = ' + str(userid) + ' AND gameid = ' + str(gameid) + ';')
    return ret != None
