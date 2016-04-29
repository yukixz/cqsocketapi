# CQSocketAPI
CoolQ Socket API written in VC++.


## Installation
0. Install .Net Framework 2.0.
1. Place `org.dazzyd.cqsocketapi.cpk` into CoolQ app folder.
2. Enable CQSocketAPI in CoolQ APP management window.
3. Restart CoolQ.


## QQBot
You can write your owned bot based on my [cqbot](https://github.com/yukixz/cqbot) in Python 3.


## Protocol

### Communication
The communication between server and clients is sending UDP frame to each other.

The server listens on port `11235`.

Clients should send `ClientHello` frame to inform server of client port.
`ClientHello` frame must be sent on startup and every (at most) 5 minutes.

A UDP frame contains one Prefix and multiple Fields.
Prefix and Fields are joined by one space.

### Server Sent Frame
```
Frame = Prefix (max 256) + Payload (max 32768)

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

EncodedText = base64_encode( GBK_encode( text ) )
```

### Client Sent Frame
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

EncodedText = base64_encode( GBK_encode( text ) )
```

### Example Frame
```
GroupMessage 123456 10000 dGVzdCCy4srU
```
