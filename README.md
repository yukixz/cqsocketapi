# CQSocketAPI
CoolQ Socket API written in VC++.

## QQBot
You can use my [cqbot](https://github.com/yukixz/cqbot) written in python 3.

## Protocol

### Communication
Server and client communicate by send UDP frame to each others. Fields are joined by one space in a frame.

The server port is 11231 and client is 11232.

### Frame
```
Frame = Prefix (max 256) + Payload (max 32768)
Prefix = 'PrivateMessage' | 'GroupMessage' | 'DiscussMessage'

Received Message Payload:
PrivateMessage = QQ + encoded_text
GroupMessage = GroupID + QQ + encoded_text
GroupMessage = DiscussID + QQ + encoded_text

Send Message Payload:
PrivateMessage = QQ + encoded_text
GroupMessage = GroupID + encoded_text
GroupMessage = DiscussID + encoded_text

encoded_text = base64_encode( GBK_encode( text ) )
```
