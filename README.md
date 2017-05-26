# CQSocketAPI
[CoolQ](http://cqp.cc/) Socket API written in VC++.

一个[酷Q](http://cqp.cc/)的 Socket 代理插件

## Installation
1. Place `org.dazzyd.cqsocketapi.cpk` into CoolQ app folder.
2. Enable CQSocketAPI in CoolQ APP management window.
3. Restart CoolQ.

## Configuration
Default `Config.ini` file will generate in `org.dazzyd.cqsocketapi` folder.


## QQBot
You can write your owned bot based on my [qqbot](https://github.com/yukixz/qqbot) in Python 3.


## Protocol
### Communication
The communication between server and clients is sending UDP frame to each other.

The server listens on port `11235`.

Clients should send `ClientHello` frame to inform server of client port.
`ClientHello` frame must be sent on startup and every (at most) 5 minutes.

A UDP frame contains one Prefix and multiple Fields.
Prefix and Fields are joined by one space.

## 安装
1. 将`org.dazzyd.cqsocketapi.cpk`复制粘贴到酷Q的app文件夹中。
2. 在酷Q应用管理界面启用对应插件。
3. 重启酷Q

## 配置
编辑Config.ini文件可以改变一些设置，文件位于`org.dazzyd.cqsocketapi`文件夹中。

```
[Server]
SERVER_PORT=11235           // 监听端口
CLIENT_SIZE=32             // 最大允许客户端数量
CLIENT_TIMEOUT=300           // 客户端超时时间（秒）
FRAME_PREFIX_SIZE=256         // 数据包前缀大小（字节）
FRAME_PAYLOAD_SIZE=32768       // 数据包数据区域大小（字节）
```

## 通讯协议
### 信息交换
插件（服务端）与客户端（您自己实现的，下同）进行UDP数据包交换。

插件默认监听本地11235端口。

客户端需要每隔至少5分钟发送一次`ClientHello`信息以保持连接。
该信息需包含客户端的端口。

注：所有文本数据需经过GB18030编码的base64转码后进行传送

单个UDP数据包由前缀与多个由空格作为间隔的区域组成。

### Server Sent Frame 服务端数据包规格
```
Frame = Prefix (max 256) + Payload (max 32768)

Prefix = 'ServerHello'
Payload = ClientTimeout + PrefixSize + PlayloadSize + FrameSize

Prefix = 'PrivateMessage'
Payload = QQ + EncodedText

Prefix = 'GroupMessage'
Payload = GroupID + QQ + EncodedText

Prefix = 'DiscussMessage'
Payload = DiscussID + QQ + EncodedText

Prefix = 'GroupMemberDecrease'
Payload = GroupID + QQ + OperatedQQ

Prefix = 'GroupMemberIncrease'
Payload = GroupID + QQ + OperatedQQ

EncodedText = base64_encode( GB18030_encode( text ) )
```

### Client Sent Frame 客户端数据包规格
```
Frame = Prefix (max 256) + Payload (max 32768)

Prefix = 'ClientHello'
Payload = Port

Prefix = 'PrivateMessage'
Payload = QQ + EncodedText

Prefix = 'GroupMessage'
Payload = GroupID + EncodedText

Prefix = 'DiscussMessage'
Payload = DiscussID + EncodedText

EncodedText = base64_encode( GB18030_encode( text ) )
```

### Example Frame 数据包范例
```
GroupMessage 123456 10000 dGVzdCCy4srU
```
