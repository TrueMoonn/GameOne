# RType project protocol documentation

# PACKET FORMAT
```
ENDIANESS : BIG
PREAMBLE : 4 BYTES : 0000
PACKETSIZE : 4 BYTES : INT  -> Size of incoming bytes to read (including code with its data)
DATETIME : NO
EOP : NO
```

---

# SENDING CODES
Every code (1 ... 255) is on ONE BYTE only

## Client codes to server

### 01 ... 19 → connexions codes
```
1   CONNEXION                               -> Indicate server that client just connected and wish to proceed
2   DISCONNEXION                            -> Sent from client to server so it can ignore/erase connexion
3   ERROR TOO MANY CLIENTS                  -> Wait and try later
4   PACKET LOSS, LAST INSTRUCTION IGNORED   -> Send back last instruction
6   PING                                    -> Send Pong
7   PONG                                    -> Calculate delay, it means client sent PING before
```

### 20 ... 29 → accounts codes
```
10  LOGIN   -> Includes a username, separated from a password
11  SIGNUP  ->
12  LOGOUT  ->
```

### 20 ... 29 → menus codes
