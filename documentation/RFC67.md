# RType Multiplayer Game Network Protocol

**RFC:** 67
**Category:** Informational  
**Date:** December 2025

---

## Status of this Memo

This memo provides information for the Internet community. This memo does not specify an Internet standard of any kind. Distribution of this memo is unlimited.

---

## Abstract

This document describes the RType Multiplayer Game Network Protocol, a client-server protocol designed for real-time multiplayer gaming. The protocol supports player authentication, lobby management, game state synchronization, and input handling for a cooperative space shooter game.

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Protocol Overview](#2-protocol-overview)
   - 2.1 [Architecture](#21-architecture)
   - 2.2 [Design Goals](#22-design-goals)
3. [Packet Format](#3-packet-format)
   - 3.1 [Header Structure](#31-header-structure)
   - 3.2 [Payload Structure](#32-payload-structure)
4. [Message Codes](#4-message-codes)
   - 4.1 [Connection Messages (1-19)](#41-connection-messages-1-19)
   - 4.2 [Account Messages (20-29)](#42-account-messages-20-29)
   - 4.3 [Lobby Messages (30-49)](#43-lobby-messages-30-49)
   - 4.4 [In-Game Messages (50-69)](#44-in-game-messages-50-69)
5. [Communication Patterns](#5-communication-patterns)
6. [Error Handling](#6-error-handling)
7. [Security Considerations](#7-security-considerations)
8. [Implementation Notes](#8-implementation-notes)

---

## 1. Introduction

The RType Multiplayer Game Network Protocol enables real-time communication between game clients and a central server in a cooperative space shooter environment. The protocol facilitates multiple players connecting, forming lobbies, and participating in synchronized gameplay sessions.

### Key Features
- Low-latency input transmission for responsive gameplay
- Efficient game state synchronization
- Lobby management with administrative controls
- Connection monitoring and error recovery

---

## 2. Protocol Overview

### 2.1. Architecture

The protocol implements a **client-server architecture** where:
- The server maintains authoritative game state
- Clients send input commands and receive state updates
- All game logic and physics calculations occur server-side
- Clients render game state based on server updates

### 2.2. Design Goals

- **Simplicity:** Fixed message codes and straightforward packet structure
- **Performance:** Minimal overhead with fixed-size fields where possible
- **Reliability:** Error detection and recovery mechanisms
- **Extensibility:** Organized message code ranges for future expansion

---

## 3. Packet Format

All packets follow a consistent structure: **fixed header + variable-length payload**.

### 3.1. Header Structure

The packet header consists of **8 bytes**:

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    PREAMBLE (0x00000000)                      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                       PACKET SIZE                             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**Fields:**
- **PREAMBLE:** 4 bytes, fixed value `0x00000000` for packet synchronization
- **PACKET SIZE:** 4 bytes (32-bit unsigned integer, big-endian), total size of payload including message code and data

### 3.2. Payload Structure

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| MESSAGE CODE  |              MESSAGE DATA                     |
+-+-+-+-+-+-+-+-+                                               |
|                       (variable length)                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**Fields:**
- **MESSAGE CODE:** 1 byte (values 1-255)
- **MESSAGE DATA:** Variable length, format depends on message code

> **Note:** All multi-byte integers use **big-endian** (network byte order).

---

## 4. Message Codes

Message codes are organized into functional ranges:

| Range | Category |
|-------|----------|
| 1-19  | Connection management |
| 20-29 | Account/authentication |
| 30-49 | Lobby management |
| 50-69 | In-game messages |

### 4.1. Connection Messages (1-19)

#### Client → Server

| Code | Name | Data | Description |
|------|------|------|-------------|
| 1 | CONNEXION | None | Initiate connection. Server responds with code 1 (CONNECTED) |
| 2 | DISCONNEXION | None | Graceful disconnect request |
| 4 | PACKET_LOSS | None | Request retransmission of last message (parsing failed) |
| 6 | PING | None | Connection probe. Server responds with code 7 (PONG) |
| 7 | PONG | None | Response to server PING |

#### Server → Client

| Code | Name | Data | Description |
|------|------|------|-------------|
| 1 | CONNECTED | None | Acknowledge successful connection |
| 2 | DISCONNEXION | None | Server-initiated disconnect |
| 3 | ERROR_TOO_MANY_CLIENTS | None | Server at capacity, client should retry later |
| 4 | PACKET_LOSS | None | Server failed to parse last client message |
| 5 | NEXT_ENTITIES | 4 bytes (int) | Next entity identifier |
| 6 | PING | None | Connection probe. Client responds with code 7 (PONG) |
| 7 | PONG | None | Response to client PING |

---

### 4.2. Account Messages (20-29)

#### Client → Server

| Code | Name | Data | Description |
|------|------|------|-------------|
| 20 | LOGIN | Variable (username) | Authenticate with username (alphabetical only). Server responds with code 22 or 25 |
| 21 | LOGOUT | None | Clear username and reset authentication |

**LOGIN format:** `[20][username bytes...]`

#### Server → Client

| Code | Name | Data | Description |
|------|------|------|-------------|
| 22 | LOGGED_IN | 4 bytes (user ID) | Confirm successful login with assigned ID |
| 25 | USERNAME_ALREADY_TAKEN | None | Requested username unavailable |

---

### 4.3. Lobby Messages (30-49)

#### Client → Server

| Code | Name | Data | Description |
|------|------|------|-------------|
| 30 | JOIN_LOBBY | 6 bytes (lobby code) | Join lobby with 6-digit code. Server responds with code 34 if invalid |
| 31 | LEAVE_LOBBY | None | Disconnect from current lobby |
| 32 | CREATE_LOBBY | None | Request new lobby. Server responds with code 33. Requester becomes admin |
| 35 | ADMIN_START_GAME | None | Start game (admin only). Server verifies and broadcasts code 36 or responds with code 37 |

#### Server → Client

| Code | Name | Data | Description | Status |
|------|------|------|-------------|--------|
| 33 | LOBBY_CREATED | 6 bytes (lobby code) | Confirm lobby creation with generated code | 🚧 WIP |
| 34 | BAD_LOBBY_CODE | None | Invalid/non-existent lobby code | 🚧 WIP |
| 36 | GAME_STARTING | None | Broadcast: game session starting |  |
| 37 | NOT_ADMIN | None | Reject ADMIN_START_GAME (insufficient privileges) | 🚧 WIP |
| 38 | PLAYERS_LIST | Variable | List of lobby players (see format below) | 🚧 WIP |
| 49 | GAME_END | None | Game session ended, return to lobby | 🚧 WIP |

**PLAYERS_LIST format:** `[38][4B ID][':'][username]['\n'][4B ID][':'][username]...`

---

### 4.4. In-Game Messages (50-69)

> **Important:** In-game state messages (51-57) use **fixed-size fields** without separators for performance.

#### Client → Server

| Code | Name | Data | Description |
|------|------|------|-------------|
| 50 | CLIENT_INPUTS | Variable (input bytes) | Player inputs as ASCII codes. Example: `[50]['z'][' ']['m']` |
| 58 | PAUSE_GAME | None | Toggle pause state. Server may broadcast code 61 |
| 59 | I_MISSED_SOMETHING | None | Request full game state. Server responds with codes 51-57 |

#### Server → Client

| Code | Name | Data Format | Description | Status |
|------|------|-------------|-------------|--------|
| 51 | PLAYERS_STATES | N×16 bytes | All player positions and health. Format: `[51][4B ID][4B X][4B Y][8B health]...` |  |
| 52 | PROJECTILES_POS | N×12 bytes | All projectile positions. Format: `[52][4B ID][4B X][4B Y]...` | 🚧 WIP |
| 53 | NEW_WAVE | 4 bytes (wave ID) | New enemy wave starting | 🚧 WIP |
| 54 | ENNEMIES_STATES | N×16 bytes | All enemy positions and health. Format: `[54][4B ID][4B X][4B Y][8B health]...` | 🚧 WIP |
| 56 | GAME_DURATION | 4 bytes (duration ms) | Elapsed time since game start | 🚧 WIP |
| 57 | GAME_LEVEL | 4 bytes (level) | Current difficulty level | 🚧 WIP |
| 60 | PLAYER_DEAD | None | Client's player eliminated | 🚧 WIP |
| 61 | GAME_PAUSED | None | Game pause state toggled | 🚧 WIP |

---

## 5. Communication Patterns

### Connection Establishment
```
Client → Server: CONNEXION (1)
Server → Client: CONNECTED (1)
Client → Server: LOGIN (20) + username
Server → Client: LOGGED_IN (22) + user ID
```

### Lobby Creation
```
Client → Server: CREATE_LOBBY (32)
Server → Client: LOBBY_CREATED (33) + lobby code
```

### Starting Game
```
Admin Client → Server: ADMIN_START_GAME (35)
Server → All Clients: GAME_STARTING (36)
```

### Gameplay Loop
```
Client → Server: CLIENT_INPUTS (50) [frequent]
Server → All Clients: PLAYERS_STATES (51) [periodic]
Server → All Clients: PROJECTILES_POS (52) [periodic]
Server → All Clients: ENNEMIES_STATES (54) [periodic]
```

### Recovery
```
Client → Server: I_MISSED_SOMETHING (59)
Server → Client: Complete state (codes 51-57)
```

---

## 6. Error Handling

| Mechanism | Description |
|-----------|-------------|
| **Packet Loss** | Code 4 (PACKET_LOSS) requests retransmission for messages with data |
| **Connection Monitoring** | PING/PONG (codes 6/7) for latency measurement and health checks |
| **State Sync** | Code 59 (I_MISSED_SOMETHING) requests full state resynchronization |
| **Validation Errors** | Codes 25 (username taken), 34 (bad lobby), 37 (not admin) |

---

## 7. Security Considerations

- **Input Validation:** Server MUST validate all inputs (usernames, data lengths)
- **Resource Limits:** Enforce connection limits (code 3 for capacity rejection)
- **Authentication:** Current protocol uses basic username identification (suitable for trusted networks)
- **Authoritative State:** Server maintains authoritative game state; client inputs are validated
- **Admin Verification:** Server MUST verify admin privileges before executing privileged commands

---

## 8. Implementation Notes

### Performance
- In-game messages designed for minimal overhead
- Fixed-size fields eliminate parsing complexity
- Batch state updates when possible

### Extensibility
- Message code ranges reserve space for future additions
- Undefined codes should be ignored for forward compatibility

### Transport Layer
- Protocol is transport-agnostic (can use TCP or UDP)
- TCP: reliability with higher latency
- UDP: lower latency, requires application-level reliability

### Byte Order
- All multi-byte integers MUST use big-endian (network byte order)

### Development Status
- Messages marked 🚧 WIP are defined but may not be fully implemented

---

## Authors

**RType Development Team**  
Email: pierre.pruvost@epitech.eu

---

*This document and the information contained herein is provided on an 'AS IS' basis.*