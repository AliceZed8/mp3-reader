# MP3 Reader
Simple MP3 Reader

## Features
- **ID3v1 Tag Reading** - Extract metadata from ID3v1 tags
- **ID3v2 Tag Reading** - Full support for ID3v2 tags and frames
- **Cover Extraction** - Automatically saves album art (APIC frames) as `output.jpg`


## Supported ID3v2 Frames

The reader supports various frame types including:
- `TIT2` - Title/Song name
- `TPE1` - Lead artist
- `TALB` - Album
- `TRCK` - Track number
- `TYER` - Year
- `APIC` - Attached picture (album art)
- And more...

## Example
```shell
./mp3-reader 'test/Yoshida Yasei - Override.mp3'
```
Output
```
Loaded MP3 file: "test/Yoshida Yasei - Override.mp3", size: 5657924 bytes
Found 1 ID3v2 tags
ID3v2 Tag: version 4.0 (113687 bytes)
Found 11 frames
- Frame: TIT2 (21 bytes)
Encoding: UTF-16
Title: Override
- Frame: TPE1 (31 bytes)
Encoding: UTF-16
Artist: Yoshida Yasei
- Frame: TRCK (11 bytes)
- Frame: TALB (21 bytes)
- Frame: TBPM (7 bytes)
- Frame: TXXX (11 bytes)
- Frame: TYER (13 bytes)
- Frame: TDAT (25 bytes)
- Frame: TSRC (29 bytes)
- Frame: TPE2 (31 bytes)
- Frame: APIC (106809 bytes)
Picture: MIME type: image/jpg, size: 106793 bytes
Saved picture as output.jpg
```

## Installation
### Prerequisites
- C compiler (gcc/clang)
- Make

### Build
```shell
make
```