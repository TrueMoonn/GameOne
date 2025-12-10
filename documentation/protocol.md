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
1   CONNEXION                               [NO DATA]   ->  Indicate server that client just connected and wish to proceed, Responded by 1
2   DISCONNEXION                            [NO DATA]   ->  Sent from client unlink/erase connexion
3   ERROR TOO MANY CLIENTS                  [NO DATA]   ->  Wait and try later
4   PACKET LOSS, LAST INSTRUCTION IGNORED   [NO DATA]   ->  Ask to send back last instruction, use only if a parsing failed for a code that contains DATA
6   PING                                    [NO DATA]   ->  Will be responded by 7
7   PONG                                    [NO DATA]   ->  Calculate delay, it means client sent PING before
```

### 20 ... 29 → accounts codes
```
20  LOGIN   [21 + X bytes username] ->  Contains a username to set for user, Will be responded by 22 or 25 (only alphabetical values)
21  LOGOUT  [NO DATA]               ->  Reset username to empty
```

### 30 ... 49 → lobby codes
```
30  JOIN LOBBY          [30 + 6 bytes lobby_code]   ->  Lobby code is a 6 numbers random value created by server
31  LEAVE LOBBY         [NO DATA]                   ->  Disconnects client from lobby
32  CREATE LOBBY        [NO DATA]                   ->  Will be responded with 33, and server needs to register admin
35  ADMIN START GAME    [NO DATA]                   ->  Sent by client, server needs to check if client is admin
```

### 50 ... 69 → in game codes
```
50  CLIENT INPUTS       [50 + X times (1B input)]   ->  Client inputs, just a list of bytes that correspond to keys pressed in ascii (ex: [50, 'z', ' ', 'm'])
58  PAUSE GAME          [NO DATA]                   ->  Player asks to pause the game / Player asks to play the game
59  I MISSED SOMETHING  [NO DATA]                   ->  Asks Server to send all game data, responded by all codes from 51 to 56 included
```


## Server codes to client

### 01 ... 19 → connexions codes
```
1   CONNECTED                               [NO DATA]   ->  Response to 1 from client
2   DISCONNEXION                            [NO DATA]   ->  Server force disconnected client
4   PACKET LOSS, LAST INSTRUCTION IGNORED   [NO DATA]   ->  Ask to send back last instruction
6   PING                                    [NO DATA]   ->  Will be responded by 7
7   PONG                                    [NO DATA]   ->  Calculate delay, it means server sent PING before
```

### 20 ... 29 → accounts codes
```
22  LOGGED IN               [NO DATA]   ->  Respond to 20 if ok
25  USERNAME ALREADY TAKEN  [NO DATA]   ->  Respond to 20 if username already taken
```

### 30 ... 49 → lobby codes
```
33  LOBBY CREATED   [33 + 6 bytes lobby_code]                   ->  Response to 32, send the created lobby code
34  BAD LOBBY CODE  [NO DATA]                                   ->  Respond to 30 if invalid code given
36  GAME STARTING   [NO DATA]                                   ->  Broadcast to all lobby clients
37  NOT ADMIN       [NO DATA]                                   ->  Send if client that sent 35 is not admin of the lobby
38  PLAYERS LIST    [38 + X times (4B id + ':' + XB username)]  ->  Send all players names, separated by \n (10)
49  GAME END        [NO DATA]                                   ->  Send when game finishes, send back all clients to lobby
```

### 50 ... 69 → in game codes
**these fields have fixed size, thus do not need parsing of any kind, and values are all packet together without separators** → `(not separated)`

```
51  PLAYERS POSITIONS   [51 + X times (4B int id + 4B int x + 4B int y)]        ->  Send all players positions (not separated)
52  PLAYERS HP          [52 + X times (4B int id + 4B int HP)]                  ->  Send all players health points (not separated)
53  PLAYERS SCORES      [53 + X times (4B int id + 4B int score)]               ->  Send all players scores (not separated)
54  ENTITIES POSITIONS  [54 + X times (4B int id + 4B int x + 4B int y)]        ->  Send all entities positions (not separated)
55  ENTITIES HP         [55 + X times (4B int id + 4B int HP)]                  ->  Send all entities health points (not separated)
56  GAME DURATION       [56 + 4B int duration]                                  ->  Send game duration since started
57  GAME LEVEL          [57 + 4B int level]                                     ->  Send current game level
60  PLAYER DEAD         [NO DATA]                                               ->  Client's player is dead :'(
61  GAME PAUSED         [NO DATA]                                               ->  A player has set the game on pause / A player has set the game on play
```
