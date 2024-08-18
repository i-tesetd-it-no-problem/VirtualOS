/**
 * @file syscall.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 用户无需修改的文件
 * @version 1.0
 * @date 2024-08-16
 * 
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */
#include <sys/stat.h>
#include <unistd.h>

int _close_r(struct _reent *r, int file) {
    return 0;
}

int _fstat_r(struct _reent *r, int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty_r(struct _reent *r, int file) {
    return 1;
}

int _getpid_r(struct _reent *r) {
    return 1;
}

int _kill_r(struct _reent *r, int pid, int sig) {
    return 0;
}

int _lseek_r(struct _reent *r, int file, int ptr, int dir) {
    return 0;
}

int _read_r(struct _reent *r, int file, char *ptr, int len) {
    return 0;
}

int _write_r(struct _reent *r, int file, char *ptr, int len) {
    return len;
}
