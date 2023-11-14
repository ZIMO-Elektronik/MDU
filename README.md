# MDU

[![build](https://github.com/ZIMO-Elektronik/MDU/actions/workflows/build.yml/badge.svg)](https://github.com/ZIMO-Elektronik/MDU/actions/workflows/build.yml) [![tests](https://github.com/ZIMO-Elektronik/MDU/actions/workflows/tests.yml/badge.svg)](https://github.com/ZIMO-Elektronik/MDU/actions/workflows/tests.yml)

<img src="data/images/logo.svg" width="15%" align="right">

MDU is an acronym for Multi Decoder Update, a protocol for [ZPP](https://github.com/ZIMO-Elektronik/ZPP) and [ZSU](https://github.com/ZIMO-Elektronik/ZSU) updates over the track. The protocol is currently supported by the following products:
- Command stations
  - [ZIMO MXULF](http://www.zimo.at/web2010/products/InfMXULF_EN.htm)
  - [Z21](https://www.z21.eu/en)
- Decoders
  - [ZIMO MN decoders](http://www.zimo.at/web2010/products/mn-nicht-sound-decoder_EN.htm)
  - [ZIMO Small-](http://www.zimo.at/web2010/products/ms-sound-decoder_EN.htm) and [large-scale MS decoders](http://www.zimo.at/web2010/products/ms-sound-decoder-grossbahn_EN.htm)

<details>
  <summary>Table of contents</summary>
  <ol>
    <li><a href="#protocol">Protocol</a></li>
      <ul>
        <li><a href="#entry">Entry</a></li>
        <li><a href="#alternative-entry">Alternative entry</a></li>
        <li><a href="#transmission">Transmission</a></li>
        <li><a href="#bit-timings">Bit timings</a></li>
        <li><a href="#structure-of-a-data-packet">Structure of a data packet</a></li>
        <li><a href="#commands">Commands</a></li>
        <li><a href="#acknowledgment">Acknowledgment</a></li>
        <li><a href="#general-commands">General commands</a></li>
        <li><a href="#zpp-commands">ZPP commands</a></li>
        <li><a href="#zsu-commands">ZSU commands</a></li>
        <li><a href="#typical-processes">Typical processes</a></li>
      </ul>
    <li><a href="#getting-started">Getting started</a></li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#configuration">Configuration</a></li>
  </ol>
</details>

## Protocol
### Entry
Activation of the MDU protocol is accomplished through a sequence of commands to **verify** configuration variables (CVs) in DCC operations mode. The entire sequence must be broadcast and thus sent to broadcast address 0. Details on the command structure can be found in [RCN-214](http://normen.railcommunity.de/RCN-214.pdf), especially point 2 ("Configuration variable access command - long form"). Depending on the type of update desired, ZPP or ZSU, the following sequences are to be sent:

| ZPP           | ZSU           |
| ------------- | ------------- |
| CV8   == 0xFE | CV8   == 0xFF |
| CV105 == 0xAA | CV105 == ID   |
| CV106 == 0x55 | CV106 == ID   |
| CV105 == 0x55 | CV105 == ID   |
| CV106 == 0xAA | CV106 == ID   |
| CV105 == 0x00 | CV105 == 0x00 |
| CV106 == 0x00 | CV106 == 0x00 |

### Alternative entry
As an alternative to entry via DCC CV verify commands, MDU commands with [default bit timings](#bit-timings) can be sent directly after switching on the track voltage. It is recommended to send out the shortest command, [Busy](#busy), for at least 200ms.

### Transmission
Bit transmission takes place MSB-first similar to the DCC protocol described in [RCN-210](http://normen.railcommunity.de/RCN-210.pdf) through zero crossings (change of polarity) of the track signal. In contrast to DCC, the transmission of a bit does not require two, but only one zero crossing. The decision as to whether a received bit represents a zero bit, one bit or acknowledgment bit is determined by the time interval between the zero crossings. This time interval has a default value at the beginning, but can be varied using a separate command (see [Config-Transfer-Rate command](#config-transfer-rate)). In addition, there are so-called fallback timings that must be able to be received at any time.

At the end of a data packet, so-called acknowledgment bits are sent by the command station. **Selected decoders** (see [Ping command](#ping)) can respond within this time by means of current pulses (ack bits). This is comparable to the programming mode of the DCC protocol (service mode) described in [RCN-216](http://normen.railcommunity.de/RCN-216.pdf). The feedback phase is divided into two channels.

**Channel 1** (ackreq bits 2-4) is intended for transmission and checksum errors (CRC). Decoders that have not completely received a packet or whose CRC check failed can declare a packet invalid by sending ack bits. command stations must then repeat the last packet sent.

**Channel 2** (ackreq bits 6-8) is for data acknowledgment. The meaning depends on the last command transmitted. For an overview, refer to the table in the [acknowledgment](#acknowledgment) chapter. The detailed description of individual commands that follows later also goes into more detail about the meaning.

Command stations must send at least 10 acknowledgment bits. Decoders that want to give feedback in a channel must answer at least 2 of 3 ackreq bits within this channel with an ack bit. Even a single received ack bit is to be evaluated by the command station as a response. In order not to overload command stations with sensitive overcurrent shutdown, the ack bits can also be transmitted as PWM instead of continuous current pulses. For Roco's Z21, for example, 90% duty cycle with a period of 10µs turned out to be ideal.

### Bit timings
At the beginning of a transfer, all devices start with the default setting. The command station can now gradually increase the transmission speed. If one of the decoders responds with an ack bit to signal that the desired speed is not supported, the station must transmit a Config-Transfer-Rate command to revise the setting with fallback timings. This is the only way to ensure that the settings on the decoders do not diverge. The fallback timings (speed 0) are therefore always active and must always be able to be received regardless of the selected speed.

| Speed        | One bit [µs] | Zero bit [µs] | Ackreq bit [µs] | Ack bit [µs] | Decoder tolerance [%] |
| ------------ | ------------ | ------------- | --------------- | ------------ | --------------------- |
| 0 (fallback) | 1200         | 2400          | 3600            | 100          | 10                    |
| 1            | 10           | 20            | 60              | 40           | 30                    |
| 2            | 20           | 40            | 60              | 40           | 20                    |
| 3            | 40           | 80            | 120             | 80           | 20                    |
| 4 (default)  | 75           | 150           | 225             | 100          | 10                    |

### Structure of a data packet
The following flowchart describes the general structure of an MDU data packet.

![alt_text](data/images/data_packet.png)

In principle, each command packet contains the phases preamble, data and acknowledgement. The meaning of the transmitted data and the acknowledgment depends on the command package itself and will be itemized later for each command.

| Command phase              | Description                                                    |
| -------------------------- | -------------------------------------------------------------- |
| Preamble                   | Identification and synchronization of an MDU packet            |
| Data (coding)              | 4-byte identification of the command                           |
| Data (optional)            | Optional transmission of data depending on the command package |
| Data (CRC)                 | 1- or 4-byte checksum of the packet                            |
| Acknowledgement (optional) | Optional acknowledgement depending on command package          |

### Commands
The supported commands of the MDU protocol are divided into 3 categories: general, ZPP and ZSU. Devices that only want to support either ZPP or ZSU updates only have to support the command set actually used. However, the general command set must be implemented.

| General commands                              | Coding      | ZPP commands                        | Coding      | ZSU commands                                        | Coding      |
| --------------------------------------------- | ----------- | ----------------------------------- | ----------- | --------------------------------------------------- | ----------- |
|                                               |             | [ZPP-Valid-Query](#zpp-valid-query) | 0xFFFF'FF06 |                                                     |             |
| [Ping](#ping)                                 | 0xFFFF'FFFF | [ZPP-LC-DC-Query](#zpp-lc-dc-query) | 0xFFFF'FF07 | [ZSU-Salsa20-IV](#zsu-salsa20-iv)              | 0xFFFF'FFF7 |
| [Config-Transfer-Rate](#config-transfer-rate) | 0xFFFF'FFFE | [ZPP-Erase](#zpp-erase)             | 0xFFFF'FF05 | [ZSU-Erase](#zsu-erase)                        | 0xFFFF'FFF5 |
| [Binary-Tree-Search](#binary-tree-search)     | 0xFFFF'FFFA | [ZPP-Update](#zpp-update)           | 0xFFFF'FF08 | [ZSU-Update](#zsu-update)                      | 0xFFFF'FFF8 |
| [CV-Read](#cv-read)                           | 0xFFFF'FFF6 | [ZPP-Update-End](#zpp-update-end)   | 0xFFFF'FF0B | [ZSU-CRC32-Start](#zsu-crc32-start)            | 0xFFFF'FFFB |
| [CV-Write](#cv-write)                         | 0xFFFF'FFF9 | [ZPP-Exit](#zpp-exit)               | 0xFFFF'FF0C | [ZSU-CRC32-Result](#zsu-crc32-result)          | 0xFFFF'FFFC |
| [Busy](#busy)                                 | 0xFFFF'FFF2 | [ZPP-Exit&Reset](#zpp-exitreset)    | 0xFFFF'FF0D | [ZSU-CRC32-Result&Exit](#zsu-crc32-resultexit) | 0xFFFF'FFFD |

### Acknowledgment
<table>
  <tr>
    <th style="text-align: center">Ackreq bit</th>
    <th style="text-align: center">0</th>
    <th style="text-align: center">1</th>
    <th style="text-align: center">2</th>
    <th style="text-align: center">3</th>
    <th style="text-align: center">4</th>
    <th style="text-align: center">5</th>
    <th style="text-align: center">6</th>
    <th style="text-align: center">7</th>
    <th style="text-align: center">8</th>
    <th style="text-align: center">9</th>
  </tr>
  <tr>
    <td><a href="#ping">Ping</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">Ping successful</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#config-transfer-rate">Config-Transfer-Rate</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">Bit timings are not supported</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#binary-tree-search">Binary-Tree-Search</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">See description</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#cv-read">CV-Read</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">Bit in CV is set</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#cv-write">CV-Write</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">Write not possible</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#busy">Busy</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error</td>
    <td></td>
    <td colspan=3 style="text-align: center">Decoder busy</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#zpp-valid-query">ZPP-Valid-Query</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">ZPP invalid</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#zpp-lc-dc-query">ZPP-LC-DC-Query</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">Load code wrong</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#zpp-erase">ZPP-Erase</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">Invalid memory area</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#zpp-update">ZPP-Update</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC32 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">Invalid address | CRC32 error</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#zpp-update-end">ZPP-Update-End</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">Invalid memory area</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#zpp-exit">ZPP-Exit</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">-</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#zpp-exitreset">ZPP-Exit&Reset</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">-</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#zsu-salsa20-iv">ZSU-Salsa20-IV</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">CRC8 error</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#zsu-erase">ZSU-Erase</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">Invalid memory area</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#zsu-update">ZSU-Update</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC32 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">Invalid address | CRC32 error</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#zsu-crc32-start">ZSU-CRC32-Start</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">Invalid memory area</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#zsu-crc32-result">ZSU-CRC32-Result</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">Received CRC32 not equal own</td>
    <td></td>
  </tr>
  <tr>
    <td><a href="#zsu-crc32-resultexit">ZSU-CRC32-Result&Exit</a></td>
    <td colspan=2 style="text-align: center">Reference</td>
    <td colspan=3 style="text-align: center">Incomplete package | CRC8 error | buffer full</td>
    <td></td>
    <td colspan=3 style="text-align: center">Received CRC32 not equal own</td>
    <td></td>
  </tr>
</table>

### General commands
The general command set contains commands for searching and selecting decoders, setting the bit timings, writing and reading configuration variables, and a busy query.

#### Ping
| Command phase   | Description                        |
| --------------- | ---------------------------------- |
| Preamble        | Identification and synchronization |
| Data (coding)   | 0xFFFF'FFFF                        |
| Data            | 4-byte serial number               |
| Data            | 4-byte decoder ID (MSB first)      |
| Data (CRC)      | 1-byte CRC8                        |
| Acknowledgement | Ping successful                    |

A ping command allows individual decoders or decoder types to be selected. Only **selected decoders** may execute commands and send acknowledgements to the command station. In the initial state after a reset, all decoders are selected. The selection is made by transmitting a 4-byte serial number and a 4-byte decoder ID. A decoder is considered to be selected if all received numbers that are not 0 match those of the decoder. If only 0 is transmitted, all decoders are selected. This results in the following variants:
- If a serial number and a decoder ID are transmitted, a single decoder with the corresponding serial number and ID is selected.
- If a serial number and decoder ID 0 is transmitted, all decoders with the corresponding serial number are selected.
- If serial number 0 and a decoder ID is transmitted, all decoders with the corresponding ID are selected.
- If serial number 0 and decoder ID 0 is transmitted, all decoders are selected.

![alt_text](data/images/ping.png)

#### Config-Transfer-Rate
| Command phase   | Description                                   |
| --------------- | --------------------------------------------- |
| Preamble        | Identification and synchronization            |
| Data (coding)   | 0xFFFF'FFFE                                   |
| Data            | 1-byte index into [bit timings](#bit-timings) |
| Data (CRC)      | 1-byte CRC8                                   |
| Acknowledgement | Bit timings are not supported                 |

With the help of a Config-Transfer-Rate command, the transmission speed can be adapted to the decoder by setting the bit timings. The exact times for one bit, zero bit, ackreq bit and ack bit can be found in the [bit timings](#bit-timings). If a decoder does not support the selected transmission speed, an acknowledgement must be sent in channel 2.

#### Binary-Tree-Search
| Command phase   | Description                        |
| --------------- | ---------------------------------- |
| Preamble        | Identification and synchronization |
| Data (coding)   | 0xFFFF'FFFA                        |
| Data            | 1-byte see description             |
| Data (CRC)      | 1-byte CRC8                        |
| Acknowledgement | See description                    |

The Binary-Tree-Search command is used to search for decoders that support MDU. The following combination of serial number and decoder ID is used for clear identification:
```c
uint64_t unique_id = (decoder_id << 32u) | serial_number;
```
With the exception of the MSB (always 0), that number can be queried bit by bit. Again, reference is made to [RCN-214](http://normen.railcommunity.de/RCN-214.pdf), which provides a similar command for the programming mode of the DCC protocol to read CVs bit by bit.

Since, in contrast to DCC, several decoders can send an acknowledgment at the same time, further commands are required in addition to querying a bit, which are represented with the help of special values or closed intervals.
- 255  
  Special value that initiates the start or restart of the search. All decoders that have received this packet reply with an acknowledgment. All decoders that have not received this packet are excluded from further search history and also send **no** acknowledgment.

- [0...62]  
  The data byte received corresponds to the bit number of the bit to be checked. All decoders with this bit set respond.

- [64...64+62]  
  The received data byte-64 corresponds to the bit number of the inverted bit to be checked. All decoders with this bit cleared respond.

- [128...128+62]  
  The received data byte-128 corresponds to the bit number of the bit to be checked. All decoders with this bit set end the search. Only special value 255 can restart the search at this point.

- [192...192+62]  
  The received data byte-192 corresponds to the bit number of the inverted bit to be checked. All decoders with this bit cleared end the search. Only special value 255 can restart the search at this point.

The following flowchart shows the search process from the perspective of the decoder. See [Maxim Integrated's application note](https://www.analog.com/en/app-notes/1wire-search-algorithm.html) [1-Wire Search Algorithm](data/1wire-search-algorithm.pdf) for more information.

![alt_text](data/images/binary_tree_search.png)

#### CV-Read
| Command phase   | Description                        |
| --------------- | ---------------------------------- |
| Preamble        | Identification and synchronization |
| Data (coding)   | 0xFFFF'FFF6                        |
| Data            | 2-byte CV number (MSB first)       |
| Data            | 1-byte bit number (0...7)          |
| Data (CRC)      | 1-byte CRC8                        |
| Acknowledgement | Bit in CV is set                   |

CV-Read reads a single bit of the configuration variable with the received number. If the bit is set, an acknowledgement is sent in channel 2.

#### CV-Write
| Command phase   | Description                        |
| --------------- | ---------------------------------- |
| Preamble        | Identification and synchronization |
| Data (coding)   | 0xFFFF'FFF9                        |
| Data            | 2-byte CV number (MSB first)       |
| Data            | 1-byte CV value                    |
| Data (CRC)      | 1-byte CRC8                        |
| Acknowledgement | Write not possible                 |

CV-Write writes a configuration variable with the received number-value pair. Any write errors must be answered with an acknowledgement in channel 2.

#### Busy
| Command phase   | Description                        |
| --------------- | ---------------------------------- |
| Preamble        | Identification and synchronization |
| Data (coding)   | 0xFFFF'FFF2                        |
| Data (CRC)      | 1-byte CRC8                        |
| Acknowledgement | Decoder busy                       |

The Busy command can be used to check whether the decoder is still busy with the last packet. If a decoder is not yet ready for a new packet, it can reply with an acknowledgment in channel 2. If the command station sends packets other than Busy to decoders that are still busy, the packets are discarded and acknowlegded with a response in channel 1.

### ZPP commands
The ZPP command set is used to update the ZPP project. It contains, among other things, an erase and update command, commands for ending the transfer and an exit command.

#### ZPP-Valid-Query
| Command phase   | Description                        |
| --------------- | ---------------------------------- |
| Preamble        | Identification and synchronization |
| Data (coding)   | 0xFFFF'FF06                        |
| Data            | 2-byte ZPP identifier              |
| Data            | 4-byte flash size                  |
| Data (CRC)      | 1-byte CRC8                        |
| Acknowledgement | ZPP invalid                        |

A ZPP-Valid-Query can be used to check whether the decoders are able to load the desired ZPP at all. The check includes the 2-character identifier of the ZPP file and the size of its flash data. If the ZPP cannot be loaded, an acknowledgment must be sent in channel 2.

#### ZPP-LC-DC-Query
| Command phase   | Description                        |
| --------------- | ---------------------------------- |
| Preamble        | Identification and synchronization |
| Data (coding)   | 0xFFFF'FF07                        |
| Data            | 4-byte developer code              |
| Data (CRC)      | 1-byte CRC8                        |
| Acknowledgement | Load code wrong                    |

A ZPP-LC-DC query can be used to check whether the decoders contain a valid load code before deleting the flash. If the received load code is not correct, an acknowledgment must be sent in channel 2.

#### ZPP-Erase
| Command phase   | Description                        |
| --------------- | ---------------------------------- |
| Preamble        | Identification and synchronization |
| Data (coding)   | 0xFFFF'FF05                        |
| Data            | 4-byte start address               |
| Data            | 4-byte end address                 |
| Data (CRC)      | 1-byte CRC8                        |
| Acknowledgement | Invalid memory area                |

With the help of ZPP-Erase, a certain memory area of the flash can be deleted. If an invalid memory area is received, an acknowledgment must be given in channel 2. :warning: **After the command, a delay of at least 3.5s must be observed.**

#### ZPP-Update
| Command phase   | Description                                 |
| --------------- | ------------------------------------------- |
| Preamble        | Identification and synchronization          |
| Data (coding)   | 0xFFFF'FF08                                 |
| Data            | 4-byte start address                        |
| Data            | N-byte payload                              |
| Data (CRC)      | 4-byte CRC32                                |
| Acknowledgement | Invalid address or CRC32 error              |

ZPP-Update is used to transfer ZPP data. If an invalid memory area or a CRC32 error is received, there must be an acknowledgment in channel 2. :warning: **Current implementations only support payloads up to 256 bytes.**

#### ZPP-Update-End
| Command phase   | Description                        |
| --------------- | ---------------------------------- |
| Preamble        | Identification and synchronization |
| Data (coding)   | 0xFFFF'FF0B                        |
| Data            | 4-byte start address               |
| Data            | 4-byte end address                 |
| Data (CRC)      | 1-byte CRC8                        |
| Acknowledgement | Invalid memory area                |

ZPP-Update-End marks the end of the current data transmission and retransmits the written memory area. Any ZPP data that is still buffered must be written when this command is received. If an invalid memory area is received, an acknowledgment must be given in channel 2.

#### ZPP-Exit
| Command phase   | Description                        |
| --------------- | ---------------------------------- |
| Preamble        | Identification and synchronization |
| Data (coding)   | 0xFFFF'FF0C                        |
| Data (CRC)      | 1-byte CRC8                        |
| Acknowledgement | -                                  |

ZPP-Exit is used to reset the decoder. The reset is only carried out if the memory area previously transferred via the ZPP-Update-End matches that written by the decoder. If this is not the case, the decoder discards all written data.

#### ZPP-Exit&Reset
| Command phase   | Description                        |
| --------------- | ---------------------------------- |
| Preamble        | Identification and synchronization |
| Data (coding)   | 0xFFFF'FF0D                        |
| Data (CRC)      | 1-byte CRC8                        |
| Acknowledgement | -                                  |

See ZPP-Exit. In addition, decoders reset their configuration variables (CV8=8).

### ZSU commands
The ZSU command set is used to update the decoder software. Among other things, it contains an update command, commands for a final CRC32 check and a command for transmitting the initialization vector of the [Salsa20](https://en.wikipedia.org/wiki/Salsa20) encryption used.

#### ZSU-Salsa20-IV
| Command phase   | Description                          |
| --------------- | ------------------------------------ |
| Preamble        | Identification and synchronization   |
| Data (coding)   | 0xFFFF'FFF7                          |
| Data            | 8-byte Salsa20 initialization vector |
| Data (CRC)      | 1-byte CRC8                          |
| Acknowledgement | CRC8 error                           |

ZSU-Salsa20-IV is used to transmit the 8-byte initialization vector of the Salsa20 encryption. :warning: **For reasons of backward compatibility, CRC8 errors must be answered in both channel 1 and channel 2.**

#### ZSU-Erase
| Command phase   | Description                        |
| --------------- | ---------------------------------- |
| Preamble        | Identification and synchronization |
| Data (coding)   | 0xFFFF'FFF5                        |
| Data            | 4-byte start address               |
| Data            | 4-byte end address                 |
| Data (CRC)      | 1-byte CRC8                        |
| Acknowledgement | Invalid memory area                |

The processor flash is deleted before an update package is written. If an invalid memory area is received, an acknowledgment must be given in channel 2. :warning: **After the command, a delay of at least 3.5s must be observed.**

#### ZSU-Update
| Command phase   | Description                        |
| --------------- | ---------------------------------- |
| Preamble        | Identification and synchronization |
| Data (coding)   | 0xFFFF'FFF8                        |
| Data            | 4-byte start address               |
| Data            | N-byte payload                     |
| Data (CRC)      | 4-byte CRC32                       |
| Acknowledgement | Invalid address or CRC32 error     |

ZSU-Update is used to transfer firmware data. If an invalid address or a CRC32 error is received, there must be an acknowledgment in channel 2. :warning: **Current implementations only support payloads of exactly 64 bytes. Smaller payloads must contain appropriate padding.**

#### ZSU-CRC32-Start
| Command phase   | Description                        |
| --------------- | ---------------------------------- |
| Preamble        | Identification and synchronization |
| Data (coding)   | 0xFFFF'FFFB                        |
| Data            | 4-byte start address               |
| Data            | 4-byte end address                 |
| Data            | 4-byte CRC32                       |
| Data (CRC)      | 1-byte CRC8                        |
| Acknowledgement | Invalid memory area                |

ZSU-CRC32-Start transfers the written memory area and the CRC32 of the encrypted firmware again at the end of the update. **It should be noted that the checksum to be compared must be calculated using the encrypted data!** If the transferred memory area does not match the one received via ZSU-Update packets, then a response must be made in channel 2. :warning: **The transferred memory area is a closed interval. The last address actually written corresponds to the end address!**

#### ZSU-CRC32-Result
| Command phase   | Description                        |
| --------------- | ---------------------------------- |
| Preamble        | Identification and synchronization |
| Data (coding)   | 0xFFFF'FFFC                        |
| Data (CRC)      | 1-byte CRC8                        |
| Acknowledgement | Received CRC32 not equal own       |

With the help of the ZSU-CRC32-Result command, the command station queries the result of the checksum previously transmitted via ZSU-CRC32-Start. If the checksum is not correct, there must be an acknowledgment in channel 2.

#### ZSU-CRC32-Result&Exit
| Command phase   | Description                        |
| --------------- | ---------------------------------- |
| Preamble        | Identification and synchronization |
| Data (coding)   | 0xFFFF'FFFD                        |
| Data (CRC)      | 1-byte CRC8                        |
| Acknowledgement | Received CRC32 not equal own       |

See ZSU-CRC32-Result. If the checksum is correct, the decoder must perform a reset.

### Typical processes
#### ZPP update
1. [Config-Transfer-Rate](#config-transfer-rate) to find a transmission speed that is supported by all decoders
2. [ZPP-Valid-Query](#zpp-valid-query)
   - [ZPP-Exit](#zpp-exit) on answer
3. [ZPP-LC-DC-Query](#zpp-lc-dc-query) (optional)
   - [ZPP-Exit](#zpp-exit) on answer
4. [ZPP-Erase](#zpp-erase)
5. [ZPP-Update](#zpp-update)
6. [ZPP-Update-End](#zpp-update-end)
7. [ZPP-Exit](#zpp-exit) | [ZPP-Exit&Reset](#zpp-exitreset)
8. Leave track voltage switched on for at least 1s

#### ZSU update
1. [Config-Transfer-Rate](#config-transfer-rate) to find a transmission speed that is supported by all decoders
2. [Binary-Tree-Search](#binary-tree-search) to find all connected decoders
3. [Ping](#ping) the desired decoders
4. [ZSU-Salsa20-IV](#zsu-salsa20-iv)
5. [ZSU-Erase](#zsu-erase)
6. [ZSU-Update](#zsu-update)
7. [ZSU-CRC32-Start](#zsu-crc32-start)
8. [ZSU-CRC32-Result](#zsu-crc32-result) | [ZSU-CRC32-Result&Exit](#zsu-crc32-resultexit)
9. Leave track voltage switched on for at least 1s

## Getting started
TODO

## Usage
To use the MDU library, a number of virtual functions must be implemented. Depending on whether ZPP or ZSU is to be transferred, one of the following abstract classes must be derived:
- mdu::rx::ZppBase
- mdu::rx::ZsuBase
- mdu::rx::ZppZsuBase

The following example shows the skeleton code for implementing the ZPP update.
```cpp
#include <mdu/mdu.hpp>

class ZppLoad : public mdu::rx::ZppBase {
public:
  // Ctor takes configuration struct containing serial number, decoder ID and
  // supported tranfer rate
  ZppLoad()
    : mdu::rx::ZppBase{{.serial_number = SERIAL_NUMBER,
                        .decoder_id = ID,
                        .transfer_rate = mdu::TransferRate::Fast}} {}

private:
  // Generate current pulse of length "us" in µs
  void ackbit(uint32_t us) const final {}

  // Read CV bit
  bool readCv(uint32_t addr, uint32_t pos) const final {}

  // Write CV
  bool writeCv(uint32_t addr, uint8_t value) final {}

  // Check if ZPP is valid
  bool zppValid(std::string_view zpp_id, size_t zpp_flash_size) const final {}

  // Check if load code is valid
  bool loadCodeValid(std::span<uint8_t const, 4uz> developer_code) const final {}

  // Erase ZPP in the closed-interval [begin_addr, end_addr[
  bool eraseZpp(uint32_t begin_addr, uint32_t end_addr) final {}

  // Write ZPP
  bool writeZpp(uint32_t addr, std::span<uint8_t const> chunk) final {}

  // Update done
  bool endZpp() final {}

  // Exit (eventually perform CV reset)
  [[noreturn]] void exitZpp(bool reset_cvs) final {}

  // Timer interrupt calls receive with captured value
  void interrupt() { receive(TIMER_VALUE); }
};
```

The entry into the MDU protocol can be handled by the `receive::entry::Point` class. The ctor takes the decoder ID and two optional function objects hooks to call before starting the ZPP or ZSU update.
```cpp
// Ctor takes ID und function object hooks with void() signature
mdu::rx::entry::Point entry_point{
  {.decoder_id = ID, .zpp_entry = enter_zpp_update, .zsu_entry = nullptr}};                                 

// Forward DCC verifies
entry_point.verify(index, value);
```

## Configuration
- MDU_UNITY_BUILD  
  Enable unity build

- MDU_RX_PREAMBLE_BITS  
  Number of preambel bits of decoder

- MDU_RX_ACKREQ_BITS  
  Number of ackreq bits of decoder

- MDU_TX_PREAMBLE_BITS  
  Number of preambel bits of command station

- MDU_TX_ACKREQ_BITS  
  Number of ackreq bits of command station

- MDU_TESTS_MASTER_KEY  
  Pass master key for ZSU decryption to tests