#define MP3_READER_IMPLEMENTATION
#include "mp3_reader.h"
#include <iconv.h>

char* utf16_to_utf8(char* input, size_t input_len) {
    size_t out_len = input_len*2;
    char *output = malloc(out_len);
    if (output == NULL) return NULL;
    
    iconv_t cd = iconv_open("UTF-8", "UTF-16");
    if (cd == (iconv_t)-1) {
        printf("iconv_open failed\n");
        free(output);
        return NULL;
    }

    char *out_ptr = output;
    char *in_ptr = input;
    if (iconv(cd, &in_ptr, &input_len, &out_ptr, &out_len) == (size_t) -1) {
        printf("iconv failed to convert\n");
        free(output);
        return NULL;
    }
    iconv_close(cd);

    *out_ptr = '\0';
    return output;
}



int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <mp3_file>\n", argv[0]);
        return 1;
    }

    MP3Reader* reader = MP3Reader_create(argv[1]);
    if (!reader) {
        return 1;
    }

    if (MP3Reader_load(reader, argv[1]) != 0) {
        MP3Reader_destroy(reader);
        return 1;
    }

    printf("Loaded MP3 file: \"%s\", size: %u bytes\n", argv[1], reader->size);

    ID3v1Tag* tag = MP3Reader_getID3v1Tag(reader);
    if (tag != NULL) {
        printf("Found ID3v1 tag:\n");
        printf("Title: %.30s\n", tag->title);
        printf("Artist: %.30s\n", tag->artist);
        printf("Album: %.30s\n", tag->album);
        printf("Year: %.4s\n", tag->year);
    }


    vector *id3v2_tags = MP3Reader_getID3v2Tags(reader);
    if (id3v2_tags != NULL) {
        printf("Found %d ID3v2 tags\n", vector_size(id3v2_tags));
        for (int i = 0; i < vector_size(id3v2_tags); i++) {
            ID3v2TagHeader* tagHeader = (ID3v2TagHeader*) vector_index(id3v2_tags, i);
            printf("ID3v2 Tag: version %d.%d (%u bytes)\n", tagHeader->version_major, tagHeader->version_minor, ID3v2Tag_getTagSize(tagHeader));

            vector* frames = MP3Reader_getID3v2TagFrames(reader, tagHeader);
            printf("Found %d frames\n", vector_size(frames));
            for (int j = 0; j < vector_size(frames); j++) {
                ID3v2TagFrameHeader* frameHeader = (ID3v2TagFrameHeader*) vector_index(frames, j);
                printf("- Frame: %s (%u bytes)\n", frameHeader->header, ID3v2Tag_getFrameSize(tagHeader, frameHeader));

                if (strncmp(frameHeader->header, "TIT2", 4) == 0) {
                    MP3_TextData *text_data = MP3Reader_getFrameTextData(tagHeader, frameHeader);
                    if (text_data != NULL) {
                        printf("Encoding: %s\n", textEncoding[text_data->encoding]);
                        printf("Title: ");
                        if (text_data->encoding == 1 || text_data->encoding == 2) { // utf-16
                            char* conv = utf16_to_utf8(text_data->data, text_data->len);
                            if (conv != NULL) printf("%s", conv);
                            free(conv);
                        } else {
                            printf("%s", text_data->data);
                        }
                        printf("\n");
                        MP3_TextData_free(text_data);
                    }
                }
                else if (strncmp(frameHeader->header, "TPE1", 4) == 0) {
                    MP3_TextData *text_data = MP3Reader_getFrameTextData(tagHeader, frameHeader);
                    if (text_data != NULL) {
                        printf("Encoding: %s\n", textEncoding[text_data->encoding]);
                        printf("Artist: ");
                        if (text_data->encoding == 1 || text_data->encoding == 2) { // utf-16
                            char* conv = utf16_to_utf8(text_data->data, text_data->len);
                            if (conv != NULL) printf("%s", conv);
                            free(conv);
                        } else {
                            printf("%s", text_data->data);
                        }
                        printf("\n");
                        MP3_TextData_free(text_data);
                    }
                }
                /*
                ... other tags
                */ 
                else if (strncmp(frameHeader->header, "APIC", 4) == 0) {
                    MP3_Picture* picture = MP3Reader_getFramePictureData(tagHeader, frameHeader);
                    if (picture != NULL) {
                        printf("Picture: MIME type: %s, size: %u bytes\n", picture->mime_type, picture->size);
                        
                        if (strcmp((char*)picture->mime_type, "image/jpeg") == 0 || strcmp((char*)picture->mime_type, "image/jpg") == 0) {
                            FILE* f = fopen("output.jpg", "wb");
                            if (f != NULL) {
                                fwrite(picture->data, 1, picture->size, f);
                                fclose(f);
                                printf("Saved picture as output.jpg\n");
                            }
                        }
                        else if (strcmp((char*)picture->mime_type, "image/png") == 0) {
                            FILE* f = fopen("output.png", "wb");
                            printf("Saved picture as output.png\n");
                        }

                        MP3_Picture_free(picture);
                    }
                }                
            }
            vector_destroy(frames);
        }
    }
    vector_destroy(id3v2_tags);
    MP3Reader_destroy(reader);
    return 0;
}