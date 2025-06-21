#include <cstddef>
#include <cstdint>
#include <cstring>

enum class Color : uint8_t {
    Black = 0, Blue, Green, Cyan, Red, Magenta, Brown, LightGrey,
    DarkGrey, LightBlue, LightGreen, LightCyan, LightRed, LightMagenta, 
	LightBrown, White,
};

constexpr size_t kWidth = 80;
constexpr size_t kHeight = 25;
constexpr uintptr_t kVideoMem = 0xB8000;

class Terminal {
public:
    Terminal()
        : row_(0), col_(0),
          color_(entryColor(Color::LightCyan, Color::Black)),
          buffer_(reinterpret_cast<uint16_t*>(kVideoMem)) {}

    void init() {
        row_ = col_ = 0;
        for (size_t y = 0; y < kHeight; ++y) {
			for (size_t x = 0; x < kWidth; ++x) {
				putAt(' ', x, y);
			}
		}
    }

    void setColor(Color fg, Color bg) {
        color_ = entryColor(fg, bg);
    }

    void putChar(char c) {
        if (c == '\n') {
            newline();
        } else {
            putAt(c, col_, row_);
            if (++col_ == kWidth) {
				newline();
			}
        }
    }
    void scrollScreen() {

        // step 1: get the memory-mapped i/o address for the vga text buffer
        char* video = (char *) kVideoMem;
        int row_size = kWidth * 2;
        
        // start from 1 because we want to move all rows up by one to start a new line
        // we start with the left most char and copy them over to the next line (hence char*)
        for (int i = 1; i < kHeight; i++) {
            char* src = video + i * row_size;
            char* dest = video + (i - 1) * row_size;
            for (int j = 0; j < row_size; j++) {
                dest[j] = src[j];
            }  
        }

        // and then since we've moved everything up already, we want to clear that last line to be blank for us to 
        // populate text with
        char* last_line = video + (kHeight - 1) * row_size;
        for (int i = 0; i < kWidth; i++) {
            last_line[i * 2] = ' ';
        }
    }
    void deleteChar() {
        if (col_ == 0 && row_ == 0) return;

        if (col_ == 0) {
            row_--;
            col_ = kWidth -1;
        } else {
            col_--;
        }
        putAt(' ', col_, row_);
    }
    
    void write(const char* data, size_t len) {
        for (size_t i = 0; i < len; ++i) {
			putChar(data[i]);
		}
    }

    void writeStr(const char* s) {
        write(s, strlen(s));
    }

private:
    size_t row_, col_;
    uint8_t color_;
    uint16_t* const buffer_;

    void newline() {
        col_ = 0;
        if (++row_ == kHeight) {
            scrollScreen();
            row_ = kHeight - 1;
        }
    }

    void putAt(char c, size_t x, size_t y) {
        const size_t idx = y * kWidth + x;
        buffer_[idx] = entry(c, color_);
    }

    static uint8_t entryColor(Color fg, Color bg) {
        return static_cast<uint8_t>(fg) | (static_cast<uint8_t>(bg) << 4);
    }

    static uint16_t entry(char c, uint8_t color) {
        return static_cast<uint16_t>(c) | (static_cast<uint16_t>(color) << 8);
    }

    static size_t strlen(const char* s) {
		const char* p = s;
		while (*p) {
			++p;
		}
		return p - s;
	}
};

extern "C" void kernelMain() {
    Terminal terminal;
    terminal.init();
    terminal.writeStr("Hello, World!\n:D");
    terminal.writeStr("Type, I echo:\n");
    extern char get_key();
    while (true) {
        char c = get_key();
        if (c) {
            if (c == '\b') {
                terminal.deleteChar();
            } else {
                terminal.putChar(c);
            }
        }
    }
}
