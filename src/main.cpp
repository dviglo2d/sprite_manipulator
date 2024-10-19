// Copyright (c) the Dviglo project
// License: MIT

#ifdef _WIN32
#include <clocale> // std::setlocale
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_WINDOWS_UTF8
#include <stb_image_write.h>

#include "dviglo/fs/fs_base.hpp"
#include "dviglo/fs/log.hpp"
#include "dviglo/std_utils/string.hpp"

using namespace dviglo;
using namespace std;


class Image
{
private:
    i32 width_;
    i32 height_;
    i32 num_components_;
    byte* data_;

    Image()
        : width_(0)
        , height_(0)
        , num_components_(0)
        , data_(nullptr)
    {
    }

    // Выделяет память
    Image(i32 width, i32 height, i32 num_components)
        : width_(width)
        , height_(height)
        , num_components_(num_components)
    {
        data_ = reinterpret_cast<byte*>(STBI_MALLOC(width_ * height_ * num_components_));
    }

    void copy_pixel(i32 src_x, i32 src_y, Image& dst, i32 dst_x, i32 dst_y)
    {
        assert(num_components_ == dst.num_components_);
        assert(src_x >= 0 && src_x < width_);
        assert(src_y >= 0 && src_y < height_);
        assert(dst_x >= 0 && dst_x < dst.width_);
        assert(dst_y >= 0 && dst_y < dst.height_);

        memcpy(dst.data_ + dst_y * dst.width_ * num_components_ + dst_x * num_components_,
               data_ + src_y * width_ * num_components_ + src_x * num_components_, num_components_);
    }

public:
    // Запрещаем копировать объект, так как если в одной из копий будет вызван деструктор,
    // все другие объекты будут иметь ссылку на уничтоженный data_
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    // Но разрешаем перемещение
    Image(Image&& other) noexcept
        : width_(std::exchange(other.width_, 0))
        , height_(std::exchange(other.height_, 0))
        , num_components_(std::exchange(other.num_components_, 0))
        , data_(std::exchange(other.data_, nullptr))
    {
    }

    ~Image()
    {
        width_ = 0;
        height_ = 0;
        num_components_ = 0;
        STBI_FREE(data_);
        data_ = nullptr;
    }

    // Загружает из файла
    static Image from_file(const char* path)
    {
        Image ret;
        ret.data_ = reinterpret_cast<byte*>(stbi_load(path, &ret.width_, &ret.height_, &ret.num_components_, 0));
        return ret;
    }

    bool empty() const { return data_ == nullptr; }

    Image expand(i32 num_pixels)
    {
        assert(num_pixels >= 0);

        Image ret(width_ + num_pixels * 2, height_ + num_pixels * 2, num_components_);

        // Верхние линии результата
        for (i32 ret_y = 0; ret_y < num_pixels + 1; ++ret_y)
        {
            for (i32 ret_x = 0; ret_x < num_pixels + 1; ++ret_x)
                copy_pixel(0, 0, ret, ret_x, ret_y);

            for (i32 src_x = 0; src_x < width_; ++src_x)
                copy_pixel(src_x, 0, ret, src_x + num_pixels, ret_y);

            for (i32 ret_x = num_pixels + width_; ret_x < ret.width_; ++ret_x)
                copy_pixel(width_ - 1, 0, ret, ret_x, ret_y);
        }

        // Все линии исходного изображения, кроме первой и последней
        for (i32 src_y = 1; src_y < height_ - 1; ++src_y)
        {
            i32 ret_y = src_y + num_pixels;

            for (i32 ret_x = 0; ret_x < num_pixels + 1; ++ret_x)
                copy_pixel(0, src_y, ret, ret_x, ret_y);

            for (i32 src_x = 0; src_x < width_; ++src_x)
                copy_pixel(src_x, src_y, ret, src_x + num_pixels, ret_y);

            for (i32 ret_x = num_pixels + width_; ret_x < ret.width_; ++ret_x)
                copy_pixel(width_ - 1, src_y, ret, ret_x, ret_y);
        }

        // Нижние линии результата
        for (i32 ret_y = ret.height_ - num_pixels - 1; ret_y < ret.height_; ++ret_y)
        {
            for (i32 ret_x = 0; ret_x < num_pixels + 1; ++ret_x)
                copy_pixel(0, height_ - 1, ret, ret_x, ret_y);

            for (i32 src_x = 0; src_x < width_; ++src_x)
                copy_pixel(src_x, height_ - 1, ret, src_x + num_pixels, ret_y);

            for (i32 ret_x = num_pixels + width_; ret_x < ret.width_; ++ret_x)
                copy_pixel(width_ - 1, height_ - 1, ret, ret_x, ret_y);
        }

        return ret;
    }

    // Сохраняет в png-файл
    void save(const char* path)
    {
        stbi_write_png(path, width_, height_, num_components_, data_, 0);
    }
};

i32 main()
{
#ifdef _WIN32
    // Меняем кодировку консоли
    setlocale(LC_CTYPE, "en_US.UTF-8");
#endif

    StrUtf8 base_path = get_base_path();
    Log log(base_path + "log.txt");

    StrUtf8 src_image_path = base_path + "src_image.png";
    StrUtf8 exp_image_path = base_path + "exp_image.png";
    i32 border_size = 2;

    Image src = Image::from_file(src_image_path.c_str());

    if (src.empty())
    {
        DV_LOG->write_error("src.empty()");
        return 1;
    }

    Image result = src.expand(border_size);
    result.save(exp_image_path.c_str());

    return 0;
}
