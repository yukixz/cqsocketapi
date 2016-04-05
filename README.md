# CQSocketAPI
CoolQ Socket API written in VC++.

## QQBot
You can use my [cqbot](https://github.com/yukixz/cqbot) written in python 3.

## Protocol

### Communication
Server and client communicate by send UDP frame to each other.
The server port is 11231 and client is 11232.

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

# TODO
* Support multiple client.