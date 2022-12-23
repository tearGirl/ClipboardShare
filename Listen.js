var Ws = require("ws"); //引入websocket模块
var child_process = require("child_process");
var SqliteDB = require('./sqlite.js').SqliteDB;
var fs = require("fs");
var iconv = require('iconv-lite');
var encodingConvert = require('encoding');
var buf = require('buffer');
var file = "answer.db";
var sqliteDB = new SqliteDB(file);

//引入net模块
const net = require('net');
//创建TCP服务器
const server = net.createServer(function(socket) {
	//监听data事件
	socket.on("data", function(data) {
		//打印数据
		var resultBuffer = encodingConvert.convert(data, "UTF8", "GBK");

		var type = resultBuffer.toString().slice(0, 1);
		var content = resultBuffer.toString().slice(1);
		if (type == "1") {
			console.log("接收到数据：" + content);
			var tileData = [
				[content]
			];
			var insertTileSql = "insert into ANSWER(CONTENT) values( ?)";
			sqliteDB.insertData(insertTileSql, tileData);
		} else if (type == "0") {
			querySql = "select CONTENT from ANSWER";
			sqliteDB.queryData(querySql, dataDeal, content);
		}
	});
	socket.on('error', (err) => {
		console.warn(err);
		socket.destroy();
	});
	socket.on('end', () => {
		console.log("有客户断开连接");
	});

	function dataDeal(objects, onecontent) {
		var result = "";
		var n = 0;
		for (let i = 0; i < objects.length; i++) {
			if (objects[i].CONTENT.includes(onecontent)) {
				result += "\n" + objects[i].CONTENT;
				n++;
			}
		}
		if (n > 0) {
			result += "\0";
			var recvBuffer = encodingConvert.convert(result, "GBK", "UTF8");
			socket.write(recvBuffer);
			console.log("发送了数据：" + recvBuffer);
		}
	}
});
//设置监听端口
server.listen(8001, function() {
	console.log("服务正在监听中。。。")
});
//监听connection事件
server.on('connection', function(socket) {
	console.log('有新的客户端接入');
});
//设置关闭时的回调函数
server.on('close', function() {
	console.log('服务已关闭');
});
//设置出错时的回调函数
server.on('error', function(err) {
	console.log('服务运行异常', err);
});
