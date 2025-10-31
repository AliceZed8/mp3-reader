#include "mp3_reader.hpp"


int main() {
    MP3Reader::MP3Reader reader;
    if (reader.load("music/Yoshida Yasei - Override.mp3")) {
        MP3Reader::MP3_Metadata meta = reader.getMetadata();
        MP3Reader::ID3v1Tag* tag = reader.getID3v1Tag();
        
        std::cout << "Title: " << meta.title << std::endl;
        std::cout << "Artist: " << meta.artist << std::endl;
        std::cout << "Album: " << meta.album << std::endl;
        std::cout << "Year: " << meta.year << std::endl;
        std::cout << "Picture mime type: " << meta.mime_type << std::endl;
        
        if (meta.image_data) {
            std::cout << "Image size: " << meta.image_size << std::endl;
            // save image
            // ...
            // NOT delete[] meta.image_data
        }
    }
}