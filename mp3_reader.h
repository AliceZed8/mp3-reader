#ifndef MP3_READER_H
#define MP3_READER_H
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>




/*
    VECTOR
*/
typedef struct vector {
    void** data;
    int size;
    int capacity;
} vector;

vector* vector_create();
void    vector_destroy(vector* v);
int     vector_size(vector* v);
int     vector_push_back(vector* v, void *item);
void*   vector_pop_back(vector *v);
void*   vector_index(vector* v, int index);





/*
    MP3
*/
#pragma pack(push, 1)
typedef struct MP3FrameHeader {
    uint32_t sync: 11;          // Sync
    uint32_t version: 2;        // MPEG version
    uint32_t layer: 2;          // Layer (I-III)
    uint32_t protection: 1;     // Protection
    uint32_t bitrate: 4;        // Bitrate
    uint32_t frequency: 2;      // Frequency
    uint32_t padding: 1;        // Padding
    uint32_t private_bit: 1;    // Private bit
    uint32_t mode: 2;           // Channel mode
    uint32_t mode_extension: 2; // Mode extension
    uint32_t copyright: 1;      // Copyright
    uint32_t original: 1;       // Original
    uint32_t emphasis: 2;       // Emphasis
} MP3FrameHeader;
#pragma pack(pop)


typedef struct ID3v1Tag {
    char header[3];      // "TAG"
    char title[30];      // Title
    char artist[30];     // Artist
    char album[30];      // Album
    char year[4];        // Year
    char comment[30];    // Comment
    uint8_t genre;       // Genre
} ID3v1Tag;



typedef struct ID3v2TagHeader {
    char    header[3];     // "ID3"
    uint8_t version_major; // Version major
    uint8_t version_minor; // Version minor
    uint8_t flags;         // Flags
    uint8_t size[4];       // Size bytes (7 bits)
} ID3v2TagHeader;


typedef struct ID3v2TagFrameHeader {
    char    header[4];     // Frame type
    uint8_t size[4];       // Size bytes (8 bits)
    uint8_t flags[2];      // Flags
} ID3v2TagFrameHeader;



// Bitrates for MPEG-1
const int bitratesMPEG1[][16] = {
    {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0}, // Layer I
    {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0},    // Layer II
    {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0}      // Layer III
};


// Bitrates for MPEG-2/2.5
const int bitratesMPEG2[][16] = {
    {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 176, 192, 224, 256, 0},    // Layer I
    {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},    // Layer II
    {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},    // Layer II
};


// Frequencies
const int frequencies[][4] = {
    {44100, 48000, 32000, 0},  // MPEG-1
    {22050, 24000, 16000, 0},  // MPEG-2
    {0,     0,     0,     0},  // Reserved
    {11025, 12000, 8000,  0}   // MPEG-2.5
};


// Versions
const char versions[][16] = {
    "MPEG 2.5\0",
    "Reserved\0",
    "MPEG 2  \0",
    "MPEG 1  \0",
};


// Channel modes
const char channelMode[][16] = {
    "Stereo\0",
    "Joint stereo\0",
    "Dual Mono\0",
    "Mono\0",
};

// Emphasis
const char emphasis[][16] = {
    "none\0",
    "50/15 ms\0",
    "Reserved\0",
    "CCIT J.17\0"
};


const char textEncoding[][16] = {
    "ISO-8859-1",
    "UTF-16",
    "UTF-16",
    "UTF-8"
};





/*
    MP3 Reader
*/

typedef struct MP3Reader {
    uint8_t* data;
    uint32_t size;
} MP3Reader;

typedef struct MP3_TextData {
    uint8_t encoding;
    char *data;
    uint32_t len;
} MP3_TextData;

typedef struct MP3_Picture {
    uint8_t mime_type[64];
    uint8_t picture_type;
    uint32_t size;
    uint8_t* data;
} MP3_Picture;


MP3Reader* MP3Reader_create(const char *filename);
void MP3Reader_destroy(MP3Reader* reader);

int MP3Reader_load(MP3Reader* reader, const char *filename);
ID3v1Tag* MP3Reader_getID3v1Tag(MP3Reader* reader);

uint32_t size7bitsToNormal(const uint8_t size[4]);
uint32_t size8bitsToNormal(const uint8_t size[4]);
uint32_t ID3v2Tag_getTagSize(ID3v2TagHeader* tagHeader);
uint32_t ID3v2Tag_getFrameSize(ID3v2TagHeader* tagHeader, ID3v2TagFrameHeader* frameHeader);

vector* MP3Reader_getID3v2Tags(MP3Reader* reader);
vector* MP3Reader_getID3v2TagFrames(MP3Reader* reader, ID3v2TagHeader* tagHeader);


MP3_TextData* MP3Reader_getFrameTextData(ID3v2TagHeader* tagHeader, ID3v2TagFrameHeader* frameHeader);
void MP3_TextData_free(MP3_TextData* data);

MP3_Picture* MP3Reader_getFramePictureData(ID3v2TagHeader* tagHeader, ID3v2TagFrameHeader* frameHeader);
void MP3_Picture_free(MP3_Picture* picture);














#ifdef MP3_READER_IMPLEMENTATION
/*
    MP3 Reader
*/

MP3Reader* MP3Reader_create(const char *filename) {
    MP3Reader* reader = (MP3Reader*) malloc(sizeof(MP3Reader));
    if (!reader) {
        printf("Failed to allocate memory for MP3Reader\n");
        return NULL;
    }
    reader->data = NULL;
    reader->size = 0;
    return reader;
}

void MP3Reader_destroy(MP3Reader* reader) {
    if (reader != NULL) {
        free(reader->data);
        free(reader);
    }
}


int MP3Reader_load(MP3Reader* reader, const char *filename) {
    if (reader == NULL) {
        printf("MP3Reader is NULL\n");
        return -1;
    }

    // free existing data
    if (reader->data != NULL) {
        free(reader->data);
        reader->data = NULL;
        reader->size = 0;
    }

    // open file
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        return -1;
    }

    // get file size
    fseek(file, 0, SEEK_END);
    reader->size = ftell(file);
    fseek(file, 0, SEEK_SET);


    // allocate memory for file data
    reader->data = (uint8_t*) malloc(reader->size);
    if (!reader->data) {
        printf("Failed to allocate memory for file data\n");
        fclose(file);
        return -1;
    }

    // read file into memory
    if (fread(reader->data, 1, reader->size, file) != reader->size) {
        printf("Failed to read file into memory\n");
        fclose(file);
        free(reader->data);
        reader->data = NULL;
        return -1;
    }

    fclose(file);
    return 0;
}




ID3v1Tag* MP3Reader_getID3v1Tag(MP3Reader* reader) {
    if (reader == NULL || reader->size < sizeof(ID3v1Tag)) {
        return NULL;
    }

    ID3v1Tag* tag = (ID3v1Tag*)(reader->data + reader->size - sizeof(ID3v1Tag));
    if (strncmp(tag->header, "TAG", 3) != 0) {
        return NULL;
    }
    return tag;
}




// Size conversion functions
uint32_t size7bitsToNormal(const uint8_t size[4]) {
    return (size[0] << 21) | (size[1] << 14) | (size[2] << 7) | size[3];
}
uint32_t size8bitsToNormal(const uint8_t size[4]) {
    return (size[0] << 24) | (size[1] << 16) | (size[2] << 8) | size[3];
}


// Get ID3v2 tag size
uint32_t ID3v2Tag_getTagSize(ID3v2TagHeader* tagHeader) {
    return size7bitsToNormal(tagHeader->size);
}


// Get ID3v2 Tag frame size
uint32_t ID3v2Tag_getFrameSize(ID3v2TagHeader* tagHeader, ID3v2TagFrameHeader* frameHeader) {
    return (tagHeader->version_major == 4) ? size7bitsToNormal(frameHeader->size) : size8bitsToNormal(frameHeader->size);
}




// Get ID3v2 Tags
vector* MP3Reader_getID3v2Tags(MP3Reader* reader) {
    vector* tags = vector_create();
    if (reader == NULL || reader->size < sizeof(ID3v2TagHeader)) {
        return tags;
    }

    for (uint32_t i = 0; i < reader->size - sizeof(ID3v2TagHeader); i++) {
        ID3v2TagHeader* header = (ID3v2TagHeader*)(reader->data + i);
        if (strncmp(header->header, "ID3", 3) == 0) {
            vector_push_back(tags, header);
        }
    }

    return tags;
}



// Get ID3v2Tag frames
vector* MP3Reader_getID3v2TagFrames(MP3Reader* reader, ID3v2TagHeader* tagHeader) {
    vector* frames = vector_create();
    if (reader == NULL || tagHeader == NULL) {
        return frames;
    }

    uint32_t start_pos = (uint8_t*)tagHeader - reader->data + sizeof(ID3v2TagHeader);
    uint32_t pos = start_pos;
    uint32_t tag_size = ID3v2Tag_getTagSize(tagHeader);

    for (; pos < start_pos + tag_size - sizeof(ID3v2TagFrameHeader); ) {
        if (pos + sizeof(ID3v2TagFrameHeader) > reader->size) break;

        ID3v2TagFrameHeader* frameHeader = (ID3v2TagFrameHeader*)(reader->data + pos);
        if (frameHeader->header[0] == 0) break; // No more frames
        vector_push_back(frames, frameHeader);

        uint32_t frame_size = ID3v2Tag_getFrameSize(tagHeader, frameHeader);
        pos += sizeof(ID3v2TagFrameHeader) + frame_size;
    }

    return frames;
}


MP3_TextData* MP3Reader_getFrameTextData(ID3v2TagHeader* tagHeader, ID3v2TagFrameHeader* frameHeader) {
    if (tagHeader == NULL || frameHeader == NULL) {
        return NULL;
    }

    MP3_TextData* text_data = (MP3_TextData*) malloc(sizeof(MP3_TextData));
    if (text_data == NULL) return NULL;

    uint8_t* frame_data = (uint8_t*)frameHeader + sizeof(ID3v2TagFrameHeader);
    uint32_t frame_size = ID3v2Tag_getFrameSize(tagHeader, frameHeader);
    uint8_t encoding = frame_data[0]; // Text encoding

    // skip encoding byte
    frame_data++;
    frame_size--;
    if (frame_size == 0) {
        MP3_TextData_free(text_data);
        return NULL;
    }

    text_data->encoding = encoding;
    text_data->data = frame_data;
    text_data->len = frame_size;
    return text_data;
}

void MP3_TextData_free(MP3_TextData* data) {
    free(data);
}




MP3_Picture* MP3Reader_getFramePictureData(ID3v2TagHeader* tagHeader, ID3v2TagFrameHeader* frameHeader) {
    if (tagHeader == NULL || frameHeader == NULL) {
        return NULL;
    }

    MP3_Picture* picture = (MP3_Picture*) malloc(sizeof(MP3_Picture));
    if (picture == NULL) return NULL;


    uint8_t* frame_data = (uint8_t*)frameHeader + sizeof(ID3v2TagFrameHeader);
    uint32_t frame_size = ID3v2Tag_getFrameSize(tagHeader, frameHeader);
    uint8_t encoding = frame_data[0]; // Text encoding
    uint32_t data_pos = 1;

    // Read MIME type
    int mime_index = 0;
    while (data_pos < frame_size && frame_data[data_pos] != 0 && mime_index < 63) {
        picture->mime_type[mime_index++] = frame_data[data_pos++];
    }
    picture->mime_type[mime_index] = '\0';
    data_pos++; // Skip null terminator

    if (mime_index == 0) {
        strncpy((char*)picture->mime_type, "image/jpeg", 64);
    }

    // picture type byte
    if (data_pos >= frame_size) {
        free(picture);
        return NULL;
    }


    picture->picture_type = frame_data[data_pos++];

    // description skip
    if (encoding == 0) {
        // ISO-8859-1
        while (data_pos < frame_size && frame_data[data_pos] != 0) {
            data_pos++;
        }
        data_pos++; // Skip null terminator
    }
    else if (encoding == 1 || encoding == 2) {
        // UTF-16
        while ((data_pos < frame_size - 1) && !(frame_data[data_pos] == 0 && frame_data[data_pos+1] == 0)) {
            data_pos += 2;
        }
        data_pos += 2; // Skip null terminator
    }
    else if (encoding == 3) {
        // UTF-8
        while (data_pos < frame_size && frame_data[data_pos] != 0) {
            data_pos++;
        }
        data_pos++; // Skip null terminator
    }
    else {
        free(picture);
        return NULL;
    }


    if (data_pos >= frame_size) {
        free(picture);
        return NULL;
    }

    // image data
    picture->size = frame_size - data_pos;
    picture->data = frame_data + data_pos;
    return picture;
}

void MP3_Picture_free(MP3_Picture* picture) {
    free(picture);
}















/*
    VECTOR
*/
vector* vector_create() {
    vector* v = (vector*) malloc(sizeof(vector));
    if (v == NULL) return NULL;

    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
    return v;
}

void vector_destroy(vector* v) {
    if (v != NULL) {
        free(v->data);
        free(v);
    }
}

int vector_size(vector* v) {
    if (v == NULL) return -1;
    return v->size;
}

int vector_push_back(vector* v, void *item) {
    if (v == NULL) return -1;

    if (v->size == v->capacity) {
        int new_cap = v->capacity == 0 ? 1 : v->capacity * 2;
        void **new_data = (void**) realloc(v->data, new_cap * sizeof(void*));
        if (new_data == NULL) return -1;

        v->data = new_data;
        v->capacity = new_cap;
    }
    v->data[v->size] = item;
    v->size++;
    return 0;
}

void* vector_pop_back(vector *v) {
    if (v == NULL || v->size == 0) return NULL;
    v->size--;
    return v->data[v->size];
}

void* vector_index(vector* v, int index) {
    if (v == NULL) return NULL;
    return v->data[index];
}




#endif // MP3_READER_IMPLEMENTATION

#endif // MP3_READER_H