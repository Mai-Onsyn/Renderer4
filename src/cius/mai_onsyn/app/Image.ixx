module;
#include <fstream>
#include <vector>
export module Image;
import Types;

#pragma pack(push, 1)

// BMP文件头
struct BMPFileHeader {
    UInt16 bfType = 0x4D42; // "BM"
    UInt32 bfSize = 0;
    UInt16 bfReserved1 = 0;
    UInt16 bfReserved2 = 0;
    UInt32 bfOffBits = 54;
};

// BMP信息头
struct BMPInfoHeader {
    UInt32 biSize = 40;
    Int32 biWidth = 0;
    Int32 biHeight = 0;
    UInt16 biPlanes = 1;
    UInt16 biBitCount = 32;
    UInt32 biCompression = 0;
    UInt32 biSizeImage = 0;
    Int32 biXPelsPerMeter = 0;
    Int32 biYPelsPerMeter = 0;
    UInt32 biClrUsed = 0;
    UInt32 biClrImportant = 0;
};

#pragma pack(pop)

export class Image {
    UInt8Buffer colorMap;
public:
    UInt32 width, height;
    Image(const UInt32 width, const UInt32 height) : width(width), height(height) {
        colorMap = makeUInt8Buffer(width * height * 4);
    }

    [[nodiscard]] UInt8* getBuffer() const {
        return colorMap.get();
    }

    void save(const String& path) const {
        std::ofstream file(path, std::ios::binary);

        if (!file.is_open()) {
            throw RuntimeError("Failed to open file: " + path);
        }

        const UInt32 imageSize = width * height * 4;

        BMPFileHeader fileHeader;
        fileHeader.bfSize = sizeof(BMPFileHeader)
            + sizeof(BMPInfoHeader)
            + imageSize;

        BMPInfoHeader infoHeader;
        infoHeader.biWidth = static_cast<Int32>(width);

        // BMP正高度表示从下往上存储
        infoHeader.biHeight = static_cast<Int32>(height);

        infoHeader.biSizeImage = imageSize;

        file.write(
            reinterpret_cast<const char*>(&fileHeader),
            sizeof(fileHeader)
        );

        file.write(
            reinterpret_cast<const char*>(&infoHeader),
            sizeof(infoHeader)
        );

        // BMP使用BGRA
        UInt8Buffer rowBuffer = makeUInt8Buffer(width * 4);

        for (Int32 y = static_cast<Int32>(height) - 1; y >= 0; --y) {
            const UInt8* src = colorMap.get() + y * width * 4;
            UInt8* dst = rowBuffer.get();

            for (UInt32 x = 0; x < width; ++x) {
                const UInt32 i = x * 4;

                // RGBA -> BGRA
                dst[i + 0] = src[i + 2];
                dst[i + 1] = src[i + 1];
                dst[i + 2] = src[i + 0];
                dst[i + 3] = src[i + 3];
            }

            file.write(
                reinterpret_cast<const char*>(dst),
                width * 4
            );
        }

        file.close();
    }
};
