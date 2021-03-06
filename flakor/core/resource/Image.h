#ifndef _FK_IMAGE_H_
#define _FK_IMAGE_H_

#include "core/opengl/texture/Texture2D.h"
#include "core/resource/Resource.h"
#include "core/resource/ResourceManager.h"

// premultiply alpha, or the effect will wrong when want to use other pixel format in Texture2D,
// such as RGB888, RGB5A1
#define FK_RGB_PREMULTIPLY_ALPHA(vr, vg, vb, va) \
    (unsigned)(((unsigned)((unsigned char)(vr) * ((unsigned char)(va) + 1)) >> 8) | \
    ((unsigned)((unsigned char)(vg) * ((unsigned char)(va) + 1) >> 8) << 8) | \
    ((unsigned)((unsigned char)(vb) * ((unsigned char)(va) + 1) >> 8) << 16) | \
    ((unsigned)(unsigned char)(va) << 24))

/**
 * @brief FK_NextPOT
 * @param x
 * @return the 'next highest' power of two
 * another way of obtaining the 'next highest' power of two:
 * while (n & (n-1)) {
 *  n = n & (n-1);
 *  }
 *
 *  n = n << 1;
 */
static int FK_NextPOT(int x)
{
    //x is a power of 2, so return directly.
    //已经是2的幂，直接返回
    if (!(x & (x-1))) {
         return (x);
     }

    x = x - 1;
    //setting all bits on the right-hand side of the most significant flagged bit to 1
    //将最高位1右边的bit位全设为1
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >>16);
    return x + 1;
}

FLAKOR_NS_BEGIN

typedef struct _MipmapInfo
{
    unsigned char* address;
    int len;
    _MipmapInfo():address(NULL),len(0){}
} MipmapInfo;

class Image : public Resource
{
public:
    //friend class TextureManager;
    /**
     * @js ctor
     */
    Image();
    /**
     * @js NA
     * @lua NA
     */
    virtual ~Image();

    /** Supported formats for Image */
    enum class Format
    {
        //! JPEG
        JPG,
        //! PNG
        PNG,
        //! TIFF
        TIFF,
        //! WebP
        WEBP,
        //! PVR
        PVR,
        //! ETC
        ETC,
        //! S3TC
        S3TC,
        //! ATITC
        ATITC,
        //! TGA
        TGA,
        //! Raw Data
        RAW_DATA,
        //! Unknown format
        UNKOWN
    };

	virtual bool load(bool async) override;
    virtual bool unload() override;
    
	/**
    @brief Load image from stream buffer.
    @param data  stream buffer which holds the image data.
    @param dataLen  data length expressed in (number of) bytes.
    @return true if loaded correctly.
    * @js NA
    * @lua NA
    */
    bool initWithImageData(const unsigned char * data, ssize_t dataLen);

    // @warning kFmtRawData only support RGBA8888
    bool initWithRawData(const unsigned char * data, ssize_t dataLen, int width, int height, int bitsPerComponent, bool preMulti = false);

    // Getters
    inline unsigned char *   getData()               { return _data; }
    inline ssize_t           getDataLen()            { return _dataLen; }
    inline Format            getFileType()           {return _fileType; }
    inline PixelFormat getRenderFormat()  { return _renderFormat; }
    inline int               getWidth()              { return _width; }
    inline int               getHeight()             { return _height; }
    inline int               getNumberOfMipmaps()    { return _numberOfMipmaps; }
    inline MipmapInfo*       getMipmaps()            { return _mipmaps; }
    inline bool              hasPremultipliedAlpha() { return _hasPremultipliedAlpha; }

    int                      getBitPerPixel();
    bool                     hasAlpha();
    bool                     isCompressed();


    /**
     @brief    Save Image data to the specified file, with specified format.
     @param    filePath        the file's absolute path, including file suffix.
     @param    isToRGB        whether the image is saved as RGB format.
     */
    bool saveToFile(const std::string &filename, bool isToRGB = true);
    
    
    /** treats (or not) PVR files as if they have alpha premultiplied.
     Since it is impossible to know at runtime if the PVR images have the alpha channel premultiplied, it is
     possible load them as if they have (or not) the alpha channel premultiplied.
     
     By default it is disabled.
     */
    static void setPVRImagesHavePremultipliedAlpha(bool haveAlphaPremultiplied);

protected:

	/*
     @brief The same result as with initWithImageFile, but thread safe. It is caused by
     loadImage() in TextureCache.cpp.
     @param fullpath  full path of the file.
     @param imageType the type of image, currently only supporting two types.
     @return  true if loaded correctly.
     */
    bool initWithImageFileThreadSafe(const std::string& fullpath);
    
    Format detectFormat(const unsigned char * data, ssize_t dataLen);
    bool isPng(const unsigned char * data, ssize_t dataLen);
    bool isJpg(const unsigned char * data, ssize_t dataLen);
    bool isTiff(const unsigned char * data, ssize_t dataLen);
    bool isWebp(const unsigned char * data, ssize_t dataLen);
    bool isPvr(const unsigned char * data, ssize_t dataLen);
    bool isEtc(const unsigned char * data, ssize_t dataLen);
    bool isS3TC(const unsigned char * data,ssize_t dataLen);
    bool isATITC(const unsigned char *data, ssize_t dataLen);

    bool initWithJpgData(const unsigned char *  data, ssize_t dataLen);
    bool initWithPngData(const unsigned char * data, ssize_t dataLen);
    bool initWithTiffData(const unsigned char * data, ssize_t dataLen);
    bool initWithWebpData(const unsigned char * data, ssize_t dataLen);
    bool initWithPVRData(const unsigned char * data, ssize_t dataLen);
    bool initWithPVRv2Data(const unsigned char * data, ssize_t dataLen);
    bool initWithPVRv3Data(const unsigned char * data, ssize_t dataLen);
    bool initWithETCData(const unsigned char * data, ssize_t dataLen);
    bool initWithS3TCData(const unsigned char * data, ssize_t dataLen);
    bool initWithATITCData(const unsigned char *data, ssize_t dataLen);
	
    typedef struct sImageTGA tImageTGA;
    bool initWithTGAData(tImageTGA* tgaData);

    bool saveImageToPNG(const std::string& filePath, bool isToRGB = true);
    bool saveImageToJPG(const std::string& filePath);
    
    void premultipliedAlpha();
    
protected:
    /**
     @brief Determine how many mipmaps can we have.
     Its same as define but it respects namespaces
     */
    static const int MIPMAP_MAX = 16;
    unsigned char *_data;
    ssize_t _dataLen;
    int _width;
    int _height;
    bool _unpack;
    Format _fileType;
    PixelFormat _renderFormat;
    MipmapInfo _mipmaps[MIPMAP_MAX];   // pointer to mipmap images
    int _numberOfMipmaps;
    // false if we cann't auto detect the image is premultiplied or not.
    bool _hasPremultipliedAlpha;

protected:
    // noncopyable
	DISALLOW_COPY_AND_ASSIGN(Image);

};

#endif

FLAKOR_NS_END
