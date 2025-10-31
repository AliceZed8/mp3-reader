#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdint>
#include <vector>


namespace MP3Reader {
    #pragma pack(push, 1)
    struct MP3FrameHeader {
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
    };
    #pragma pack(pop)



    struct ID3v1Tag {
        char header[3];      // "TAG"
        char title[30];      // Title
        char artist[30];     // Artist
        char album[30];      // Album
        char year[4];        // Year
        char comment[30];    // Comment
        uint8_t genre;       // Genre
    };


    struct ID3v2TagHeader {
        char    header[3];     // "ID3"
        uint8_t version_major; // Version major
        uint8_t version_minor; // Version minor
        uint8_t flags;         // Flags
        uint8_t size[4];       // Size bytes (7 bits)
    };

    struct ID3v2TagFrameHeader {
        char    header[4];     // Frame type
        uint8_t size[4];       // Size bytes (8 bits)
        uint8_t flags[2];      // Flags
    };


    struct MP3_Metadata {
        std::string title;
        std::string artist;
        std::string album;
        std::string year;
        std::string track_num;
        std::string genre;

        std::string mime_type;
        std::string description;
        
        uint8_t* image_data;
        uint32_t image_size;
    };


    // Bitrates for MPEG-1
    static const int bitratesMPEG1[][16] = {
        {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0}, // Layer I
        {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0},    // Layer II
        {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0}      // Layer III
    };


    // Bitrates for MPEG-2/2.5
    static const int bitratesMPEG2[][16] = {
        {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 176, 192, 224, 256, 0},    // Layer I
        {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},    // Layer II
        {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},    // Layer II
    };

    // Frequencies
    static const int frequencies[][4] = {
        {44100, 48000, 32000, 0},  // MPEG-1
        {22050, 24000, 16000, 0},  // MPEG-2
        {0,     0,     0,     0},  // Reserved
        {11025, 12000, 8000,  0}   // MPEG-2.5
    };

    // Versions
    static const char versions[][16] = {
        "MPEG 2.5\0",
        "Reserved\0",
        "MPEG 2  \0",
        "MPEG 1  \0",
    };

    // Channel modes
    static const char channelMode[][16] = {
        "Stereo\0",
        "Joint stereo\0",
        "Dual Mono\0",
        "Mono\0",
    };

    // Emphasis
    static const char emphasis[][16] = {
        "none\0",
        "50/15 ms\0",
        "Reserved\0",
        "CCIT J.17\0"
    };

    class MP3Reader {
    public:
        MP3Reader(): data(nullptr), size(0) {

        }

        ~MP3Reader() {
            if (data) delete[] data;
        }


        // Load and save in memory
        bool load(const std::string& filename) {
            std::ifstream file(filename, std::ios::in | std::ios::binary);
            if (!file.is_open()) {
                std::cerr << "Failed to open file" << std::endl;
                return false;
            }


            // Get size
            file.seekg(0, std::ios::end);
            size = file.tellg();
            if (size == -1) {
                std::cerr << "Failed to get file length" << std::endl;
                file.close();
                return false;
            }
            file.seekg(0, std::ios::beg);

            // Allocate memory
            data = new uint8_t[size];
            if (!data) {
                std::cerr << "Failed to allocate memory" << std::endl;
                file.close();
                return false;
            }

            // Read
            if (!file.read(reinterpret_cast<char*>(data), size)) {
                std::cerr << "Failed to read file into memory" << std::endl;
                file.close();
                delete[] data;
                data = nullptr;
                return false;
            }

            file.close();        
            return true;
        }



        // Test
        void printFirstFrame() {
            uint8_t* frame = getFirstFramePtr();
            if (!frame) return;

            MP3FrameHeader* header = reinterpret_cast<MP3FrameHeader*>(frame);


            uint32_t bitrate = header->version == 3 ?  bitratesMPEG1[3-header->layer][header->bitrate] : bitratesMPEG2[3-header->layer][header->bitrate];
            uint32_t frequency = frequencies[3 - header->version][header->frequency];

            std::cout << "Version: " << versions[header->version] << std::endl;
            std::cout << "Layer: " << 3 - header->layer + 1 << std::endl;
            std::cout << "Protected: " << !header->protection << std::endl;
            std::cout << "Bitrate: " << bitrate << std::endl;
            std::cout << "Frequency: " << frequency << std::endl;
            std::cout << "Padding: " << header->padding << std::endl;
            std::cout << "Mode: " <<  channelMode[header->mode] << std::endl;
            std::cout << "Copyright: " << header->copyright << std::endl;
            std::cout << "Original: " << header->original << std::endl;
            std::cout << "Emphasis: " << emphasis[header->emphasis] << std::endl;

            bitrate *= 1000;
                                    // layer I
            uint32_t frame_size = header->layer == 3 ? ((12 * bitrate / frequency) + header->padding) * 4 : (144 * bitrate / frequency) + header->padding;
            std::cout << "Frame size: " << frame_size << std::endl;
        }


        // Get ID3v1Tag ptr
        ID3v1Tag* getID3v1Tag() {
            if (size < sizeof(ID3v1Tag)) {
                std::cout << "File too small for ID3v1 tag" << std::endl;
                return nullptr;
            }


            ID3v1Tag* tag = reinterpret_cast<ID3v1Tag*>(data + size - sizeof(ID3v1Tag));
            char tag_id[4] = {0};
            std::memcpy(tag_id, tag, 3);

            if (std::strcmp(tag_id, "TAG") == 0) return tag;

            return nullptr;
        }


        // Get ID3v2 Tags
        std::vector<ID3v2TagHeader*> getID3v2Tags() {
            std::vector<ID3v2TagHeader*> tags;
            for (int i = 0; i < size - sizeof(ID3v2TagHeader); i++) {
                if (isID3v2TagHeader(data + i)) {
                    tags.push_back(reinterpret_cast<ID3v2TagHeader*>(data+i));
                }
            }

            return tags;
        }

        // Get ID3v2Tag frames
        std::vector<ID3v2TagFrameHeader*> getID3v2TagFrames(ID3v2TagHeader* tagHeader) {
            std::vector<ID3v2TagFrameHeader*> frames;
            if (!tagHeader) return frames;

            uint32_t start_pos = (reinterpret_cast<uint8_t*>(tagHeader) - data) + sizeof(ID3v2TagHeader);
            uint32_t pos = start_pos;
            uint32_t tag_size = getTagSize(tagHeader);

            while (pos < start_pos + tag_size - sizeof(ID3v2TagFrameHeader)) {
                if (pos + sizeof(ID3v2TagFrameHeader) > size) break;

                ID3v2TagFrameHeader* frameHeader = reinterpret_cast<ID3v2TagFrameHeader*>(data + pos);
                if (frameHeader->header[0] == 0) break;
                frames.push_back(frameHeader);

                uint32_t frame_size = getFrameSize(tagHeader, frameHeader);
                pos += sizeof(ID3v2TagFrameHeader) + frame_size;
            }

            return frames;
        }

        // Get frame data
        uint8_t* getFrameData(ID3v2TagFrameHeader* frameHeader) {
            if (!frameHeader) return nullptr;
            uint8_t* frame_data = reinterpret_cast<uint8_t*>(frameHeader) + sizeof(ID3v2TagFrameHeader);
            return frame_data;
        }


        // Get Metadata
        MP3_Metadata getMetadata() {
            MP3_Metadata metadata;
            char frame_id[5] = {0};

            auto tags = getID3v2Tags();
            for (auto tag: tags) {
                auto tag_frames = getID3v2TagFrames(tag);
                for (auto frame: tag_frames) {
                    std::memcpy(frame_id, frame->header, 4);
                    if      (strcmp(frame_id, "TIT2") == 0) metadata.title = getTextFrameData(tag, frame);
                    else if (strcmp(frame_id, "TPE1") == 0) metadata.artist = getTextFrameData(tag, frame);
                    else if (strcmp(frame_id, "TALB") == 0) metadata.album = getTextFrameData(tag, frame);
                    else if (strcmp(frame_id, "TYER") == 0 || strcmp(frame_id, "TDRC") == 0) metadata.year = getTextFrameData(tag, frame);
                    else if (strcmp(frame_id, "TRCK") == 0) metadata.track_num = getTextFrameData(tag, frame);
                    else if (strcmp(frame_id, "TCON") == 0) metadata.genre = getTextFrameData(tag, frame);
                    else if (strcmp(frame_id, "APIC") == 0) {
                        uint8_t* image_data = getPictureFrameData(tag, frame, metadata.mime_type, metadata.image_size);

                        // copy picture
                        // metadata.image_data = new uint8_t[metadata.image_size];
                        // if (!metadata.image_data) {
                        //     std::cerr << "Failed to allocate memory for picture" << std::endl;
                        //     metadata.image_data = nullptr;
                        // } else {
                        //     std::memcpy(metadata.image_data, image_data, metadata.image_size);
                        // }

                        metadata.image_data = image_data;
                    }
                }
            }
            return metadata;
        }


    private:
        
        // Frame
        uint8_t* getFirstFramePtr(uint32_t pos = 0) {
            for (int i = pos; i < size - 4; i++) {
                if (isFrameHeader(data + i)) {
                    return data + i;
                }
            }

            return nullptr;
        }

        bool isFrameHeader(uint8_t* data) {
            MP3FrameHeader* header = reinterpret_cast<MP3FrameHeader*>(data);
            
            if (header->sync != 0x7FF) return false;
            if (header->version == 1) return false;
            if (header->layer == 0) return false;
            if (header->bitrate == 0xF) return false;
            if (header->frequency == 3) return false;
            
            return true;
        }




        uint32_t size7bitsToNormal(const uint8_t size[4]) {
            return (size[0] << 21) | (size[1] << 14) | (size[2] << 7) | size[3];
        }
        uint32_t size8bitsToNormal(const uint8_t size[4]) {
            return (size[0] << 24) | (size[1] << 16) | (size[2] << 8) | size[3];
        }

        uint32_t getTagSize(ID3v2TagHeader* tagHeader) {
            return size7bitsToNormal(tagHeader->size);
        }
        uint32_t getFrameSize(ID3v2TagHeader* tagHeader, ID3v2TagFrameHeader* frameHeader) {
            return (tagHeader->version_major == 4) ? size7bitsToNormal(frameHeader->size) : size8bitsToNormal(frameHeader->size);
        }



        // Check ID3v2Tag header
        bool isID3v2TagHeader(uint8_t* data) {
            ID3v2TagHeader* header = reinterpret_cast<ID3v2TagHeader*>(data);
            if (header->header[0] == 'I' && header->header[1] == 'D' && header->header[2] == '3') return true;

            return false;
        }

        // Get text in frame
        std::string getTextFrameData(ID3v2TagHeader* tagHeader, ID3v2TagFrameHeader* frameHeader) {
            std::string result;
            if (!frameHeader) return result;
            uint8_t* frame_data = getFrameData(frameHeader);
            uint32_t frame_size = getFrameSize(tagHeader, frameHeader);
            uint8_t encoding = frame_data[0]; 

            // skip encoding
            frame_data++;
            frame_size--;
            

            if (encoding == 0) {
                // ISO-8859-1
                return std::string(reinterpret_cast<char*>(frame_data), frame_size);
            }
            else if (encoding == 1 || encoding == 2) {
                // UTF-16
                for (uint32_t i = 0; i < frame_size; i += 2) {
                    if (frame_data[i] != 0 && frame_data[i] < 128) {
                        result += static_cast<char>(frame_data[i]);
                    }
                }
                return result;
            }
            else if (encoding == 3) {
                // UTF-8
                return std::string(reinterpret_cast<char*>(frame_data), frame_size);
            }

            
            
            return result;
        }

        // Get picture data in frame
        uint8_t* getPictureFrameData(ID3v2TagHeader* tagHeader, ID3v2TagFrameHeader* frameHeader, std::string& mime_type, uint32_t& image_size) {
            uint8_t* frame_data = getFrameData(frameHeader);
            uint32_t frame_size = getFrameSize(tagHeader, frameHeader);
            uint8_t encoding = frame_data[0]; 
            uint32_t data_pos = 1;
            
            // Mime type
            mime_type.clear();
            while (data_pos < frame_size && frame_data[data_pos] != 0) {
                mime_type += frame_data[data_pos++];
            }
            data_pos++;
            if (mime_type.empty()) mime_type = "image/jpeg";
            


            // Picture type skip (1 byte)
            data_pos++;


            // skip info
            if (encoding == 0) {
                // ISO-8859-1
                while (data_pos < frame_size && frame_data[data_pos] != 0) {
                    data_pos++;
                }
                data_pos++;
            }
            else if (encoding == 1 || encoding == 2) {
                // UTF-16
                while (data_pos < frame_size - 1 && !(frame_data[data_pos] == 0 && frame_data[data_pos+1] == 0)) {
                    data_pos += 2;
                }
                data_pos += 2;
            }
            else if (encoding == 3) {
                // UTF-8
                while (data_pos < frame_size && frame_data[data_pos] != 0) {
                    data_pos++;
                }
                data_pos++;
            }

            // Image data
            image_size = frame_size - data_pos;
            uint8_t* image_data = frame_data + data_pos;
            return image_data;
        }
        


        uint8_t* data;
        int size;
    };
};