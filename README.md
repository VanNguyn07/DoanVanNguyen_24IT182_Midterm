Midterm Project – Implement ls(1)

Chương trình triển khai phiên bản rút gọn của lệnh UNIX ls bằng ngôn ngữ C, dựa trên manual ls.pdf được cung cấp trong đề bài.

1. Giới thiệu

C# Midterm Project – Implement ls(1)

[![Language](https://img.shields.io/badge/Language-C99-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![Platform](https://img.shields.io/badge/Platform-NetBSD%2010.1-orange.svg)](https://www.netbsd.org/)
[![Compiler](https://img.shields.io/badge/Compiler-GCC%2010.5.0-green.svg)](https://gcc.gnu.org/)
[![Build Tool](https://img.shields.io/badge/Build-Make-lightgrey.svg)](https://www.gnu.org/software/make/)

> Chương trình triển khai phiên bản rút gọn của lệnh UNIX `ls` bằng ngôn ngữ C, dựa trên manual `ls.pdf` được cung cấp trong đề bài.

---

## 📑 Mục lục
1. [Giới thiệu](#1-giới-thiệu)
2. [Chức năng](#2-chức-năng)
   - [Hành vi cơ bản](#hành-vi-cơ-bản)
   - [Các tùy chọn đã tích hợp](#các-tùy-chọn-đã-tích-hợp)
   - [Quy tắc ghi đè tùy chọn](#quy-tắc-ghi-đè-tùy-chọn)
   - [Thông tin chi tiết (Long format)](#thông-tin-chi-tiết)
   - [Biến môi trường & Xử lý I/O](#biến-môi-trường)
3. [Yêu cầu môi trường](#3-yêu-cầu-môi-trường)
4. [Cấu trúc mã nguồn](#4-cấu-trúc-mã-nguồn)
5. [Biên dịch](#5-biên-dịch)
6. [Hướng dẫn sử dụng](#6-hướng-dẫn-sử-dụng)
7. [Tài liệu tham khảo](#7-tài-liệu-tham-khảo)

---

## 1. Giới thiệu

* **Mục tiêu:** Chương trình liệt kê file và thư mục, hiển thị metadata, sắp xếp kết quả và duyệt thư mục con theo các tùy chọn được yêu cầu.
* **Cấu trúc mã nguồn:** Được chia thành nhiều module `.c` và header `.h`. Sử dụng `Makefile` để tự động hóa biên dịch từng module, sau đó liên kết thành file thực thi `ls`.
* **Phạm vi triển khai:** Tập trung vào tập chức năng trong manual `ls.pdf` được cung cấp trong đề bài, không phải toàn bộ tùy chọn của lệnh `ls` thực tế trên hệ thống.

---

## 2. Chức năng

### Hành vi cơ bản
* **Không truyền đường dẫn:** Liệt kê nội dung của thư mục hiện tại (`.`).
* **Truyền file:** Hiển thị file và thông tin được yêu cầu.
* **Truyền thư mục:** Liệt kê nội dung bên trong thư mục.
* **Hỗ trợ nhiều đường dẫn:** Xử lý nhiều đối số trong một lần gọi; luôn hiển thị các đối số không phải thư mục trước, sau đó mới xử lý các thư mục.
* **Định dạng mặc định:** Hiển thị mỗi mục trên một dòng và sắp xếp theo thứ tự bảng chữ cái (alphabetical).
* **Quản lý lỗi:** 
  * Báo lỗi khi gặp đường dẫn không tồn tại hoặc thao tác filesystem thất bại.
  * Tiếp tục xử lý các đối số hợp lệ còn lại khi một đối số bị lỗi.

### Các tùy chọn đã tích hợp

| Tùy chọn | Chức năng chi tiết |
| :---: | :--- |
| `-a` | Hiển thị tất cả mục, kể cả `.` và `..` |
| `-A` | Hiển thị mục ẩn nhưng loại trừ `.` và `..`; mặc định bật khi chạy với quyền root |
| `-c` | Chọn thời gian thay đổi trạng thái file (ctime) để hiển thị với `-l` hoặc sắp xếp với `-t` |
| `-d` | Hiển thị bản thân thư mục, không liệt kê nội dung; không đi theo symbolic link ở đối số |
| `-F` | Thêm ký hiệu nhận biết loại file sau tên (`/`, `*`, `@`, `=`, `\|`,...) |
| `-f` | Không sắp xếp (giữ nguyên thứ tự đọc từ filesystem) |
| `-h` | Hiển thị kích thước dễ đọc (K, M, G,...) khi dùng với `-l` và `-s` |
| `-i` | Hiển thị số inode |
| `-k` | Dùng đơn vị 1024 byte cho số block |
| `-l` | Hiển thị thông tin chi tiết (long format) |
| `-n` | Hiển thị chi tiết tương tự `-l` nhưng với UID/GID dạng số |
| `-q` | Thay ký tự không in được trong tên bằng `?` |
| `-R` | Liệt kê thư mục con theo chiều sâu (đệ quy) |
| `-r` | Đảo thứ tự sắp xếp |
| `-S` | Sắp xếp theo kích thước, lớn nhất trước |
| `-s` | Hiển thị số block thực tế được cấp phát |
| `-t` | Sắp xếp theo thời gian, mới nhất trước |
| `-u` | Chọn thời gian truy cập (atime) để hiển thị với `-l` hoặc sắp xếp với `-t` |
| `-w` | Hiển thị tên nguyên dạng |

### Quy tắc ghi đè tùy chọn
* **Cặp tùy chọn ưu tiên lựa chọn xuất hiện cuối cùng:**
  * `-l` và `-n`
  * `-c` và `-u`
  * `-R` và `-d`
  * `-k` và `-h`
  * `-q` và `-w`
* **Quy tắc với `-f`:** Khi bật `-f`, chương trình giữ thứ tự đọc được và không áp dụng việc đảo thứ tự sắp xếp (`-r`).

### Thông tin chi tiết (Long format)
Khi kích hoạt `-l` hoặc `-n`, mỗi dòng hiển thị gồm:
1. Loại file và quyền truy cập (permissions).
2. Số hard link.
3. Chủ sở hữu (User) và nhóm (Group) — hiển thị ID dạng số nếu không tìm được tên.
4. Kích thước byte hoặc số major/minor của file thiết bị.
5. Thời gian được chọn (`mtime`, `atime`, hoặc `ctime`).
6. Tên file.
7. Đích symbolic link sau ký hiệu `->` (nếu là symbolic link).

### Biến môi trường & Xử lý I/O

| Biến môi trường | Ảnh hưởng |
| :--- | :--- |
| `BLOCKSIZE` | Đơn vị hiển thị số block khi không dùng `-k` hoặc `-h` |
| `TZ` | Múi giờ khi hiển thị thời gian |
| `LC_ALL` / `LANG` | Locale dùng để định dạng thời gian và nhận biết ký tự trong tên |

> [!NOTE]
> * **Output tới terminal:** Mặc định thay ký tự không in được bằng dấu `?` (tương đương `-q`).
> * **Output chuyển hướng (pipe/redirect):** Mặc định giữ tên nguyên dạng (tương đương `-w`).

---

## 3. Yêu cầu môi trường

| Thành phần | Đặc tả / Giá trị |
| :--- | :--- |
| **Hệ điều hành** | NetBSD 10.1 |
| **Trình biên dịch** | `cc`, GCC 10.5.0 |
| **Chuẩn ngôn ngữ** | C99 |
| **Công cụ build** | `make` |
| **Shell** | `/bin/sh` |

Kiểm tra thông số môi trường trước khi chạy:
```bash
uname -sr
cc --version
command -v makehương trình liệt kê file và thư mục, hiển thị metadata, sắp xếp kết quả và duyệt thư mục con theo các tùy chọn được yêu cầu.

Mã nguồn được chia thành nhiều module .c và header .h. Makefile thực hiện biên dịch từng module, sau đó liên kết thành file thực thi ls.

Phạm vi bài làm là tập chức năng trong manual được cung cấp, không phải toàn bộ tùy chọn của lệnh ls thực tế.

2. Chức năng

Hành vi cơ bản

Không truyền đường dẫn: liệt kê nội dung thư mục hiện tại.

Truyền file: hiển thị file và thông tin được yêu cầu.

Truyền thư mục: liệt kê nội dung thư mục.

Hỗ trợ nhiều đường dẫn trong một lần gọi.

Hiển thị các đối số không phải thư mục trước, sau đó xử lý thư mục.

Mặc định hiển thị mỗi mục trên một dòng và sắp xếp theo tên.

Báo lỗi khi gặp đường dẫn không tồn tại hoặc thao tác filesystem thất bại.

Tiếp tục xử lý các đối số hợp lệ còn lại khi một đối số bị lỗi.

Các tùy chọn đã tích hợp

Tùy chọn

Chức năng

-a

Hiển thị tất cả mục, kể cả . và ..

-A

Hiển thị mục ẩn nhưng loại trừ . và ..; mặc định bật khi chạy với quyền root

-c

Chọn thời gian thay đổi trạng thái file để hiển thị với -l hoặc sắp xếp với -t

-d

Hiển thị bản thân thư mục, không liệt kê nội dung; không đi theo symbolic link ở đối số

-F

Thêm ký hiệu nhận biết loại file sau tên

-f

Không sắp xếp

-h

Hiển thị kích thước dễ đọc với -l và -s

-i

Hiển thị số inode

-k

Dùng đơn vị 1024 byte cho số block

-l

Hiển thị thông tin chi tiết

-n

Hiển thị chi tiết với UID/GID dạng số

-q

Thay ký tự không in được trong tên bằng ?

-R

Liệt kê thư mục con theo chiều sâu

-r

Đảo thứ tự sắp xếp

-S

Sắp xếp theo kích thước, lớn nhất trước

-s

Hiển thị số block thực tế được cấp phát

-t

Sắp xếp theo thời gian, mới nhất trước

-u

Chọn thời gian truy cập để hiển thị với -l hoặc sắp xếp với -t

-w

Hiển thị tên nguyên dạng

Các cặp tùy chọn sau sử dụng lựa chọn xuất hiện cuối cùng:

-l và -n.

-c và -u.

-R và -d.

-k và -h.

-q và -w.

Khi bật -f, chương trình giữ thứ tự đọc được và không áp dụng việc đảo thứ tự sắp xếp.

Thông tin chi tiết

Long format hiển thị:

Loại file và quyền truy cập.

Số hard link.

Chủ sở hữu và nhóm.

Kích thước byte hoặc số major/minor của file thiết bị.

Thời gian được chọn.

Tên file.

Đích symbolic link sau ký hiệu ->.

Nếu không tìm được tên chủ sở hữu hoặc nhóm, chương trình hiển thị ID dạng số.

Biến môi trường

Biến

Ảnh hưởng

BLOCKSIZE

Đơn vị hiển thị số block khi không dùng -k hoặc -h

TZ

Múi giờ khi hiển thị thời gian

LC_ALL / LANG

Locale dùng để định dạng thời gian và nhận biết ký tự trong tên

Output tới terminal mặc định thay ký tự không in được bằng ?. Output chuyển hướng mặc định giữ tên nguyên dạng.

3. Yêu cầu môi trường

Môi trường đã sử dụng:

Thành phần

Giá trị

Hệ điều hành

NetBSD 10.1

Trình biên dịch

cc, GCC 10.5.0

Chuẩn ngôn ngữ

C99

Công cụ build

make

Shell

/bin/sh

Kiểm tra môi trường:

uname -sr
cc --version
command -v make

Chương trình sử dụng một số API của NetBSD, bao gồm getbsize() và humanize_number(). Chưa xác nhận khả năng biên dịch nguyên trạng trên các hệ điều hành khác.

4. Cấu trúc mã nguồn

File nguồn

Nhiệm vụ

main.c

Thiết lập locale, tiếp nhận tham số và điều phối

options.c

Phân tích tùy chọn và xác định các mục được hiển thị

operands.c

Kiểm tra, phân nhóm và xử lý đường dẫn đầu vào

listing.c

Đọc và hiển thị nội dung thư mục

sorting.c

Sắp xếp theo tên, kích thước hoặc thời gian

traversal.c

Duyệt cây thư mục bằng stack

display.c

Hiển thị inode, block, tên và dòng tổng

mode.c

Định dạng quyền truy cập và ký hiệu loại file

long_format.c

Hiển thị thông tin chi tiết và đích symbolic link

sizes.c

Xử lý đơn vị block, BLOCKSIZE và kích thước dễ đọc

timestamps.c

Chọn thời gian sửa nội dung, truy cập hoặc thay đổi trạng thái

names.c

Xử lý tên theo -q và -w

Các module có header .h tương ứng, trừ main.c.

Các file hỗ trợ:

File

Mục đích

Makefile

Biên dịch và liên kết chương trình

.gitignore

Loại trừ sản phẩm biên dịch và file tạm khỏi Git

tests/test_ls.sh

Kiểm thử đối chiếu với /bin/ls

tests/results.txt

Kết quả kiểm thử đã ghi nhận

5. Biên dịch

Mở terminal trong thư mục dự án:

make

Makefile tạo các file object .o, sau đó liên kết thành file thực thi:

ls

Các cờ biên dịch:

-std=c99 -Wall -Wextra -Wpedantic -g

Xóa sản phẩm biên dịch:

make clean

Biên dịch lại từ đầu:

make clean
make

6. Sử dụng

Liệt kê cơ bản

./ls
./ls /home/doanvannguyen
./ls main.c .gitignore

File ẩn và thông tin chi tiết

./ls -a
./ls -A
./ls -l
./ls -n
./ls -li

Thư mục và symbolic link

./ls -d test_cases
./ls -R test_cases
./ls -l test_cases/link_dir
./ls -F test_cases

Sắp xếp

./ls -r test_cases
./ls -S test_cases
./ls -t test_cases
./ls -tu test_cases
./ls -tc test_cases
./ls -f test_cases

Kích thước và đơn vị block

./ls -s test_cases
./ls -lsk test_cases
./ls -lsh test_cases
BLOCKSIZE=2048 ./ls -ls test_cases

-k chỉ đổi đơn vị đếm block; kích thước byte trong long format vẫn giữ nguyên. -h định dạng cả kích thước nội dung và dung lượng được cấp phát.

Thứ tự tùy chọn

./ls -ln main.c
./ls -nl main.c

./ls -Rd test_cases
./ls -dR test_cases

./ls -lshk test_cases
./ls -lskh test_cases

Ví dụ, -ln dùng UID/GID dạng số, còn -nl dùng tên chủ sở hữu và nhóm.

Múi giờ và tên file

TZ=UTC ./ls -l test_cases
TZ=Asia/Ho_Chi_Minh ./ls -l test_cases

./ls -q test_cases/name_tests
./ls -w test_cases/name_tests

Chạy thông qua PATH

Có thể đặt bản riêng vào thư mục cá nhân với tên myls:

mkdir -p ~/bin
cp ./ls ~/bin/myls

Nếu ~/bin có trong PATH:

myls -l test_cases

Trong các phép so sánh, dùng /bin/ls để gọi rõ bản hệ thống.

6. Tài liệu tham khảo

Manual ls.pdf được cung cấp trong đề bài.

Tài liệu API NetBSD dùng trong quá trình triển khai: stat(2), readlink(2), getopt(3), opendir(3), readdir(3), getbsize(3) và humanize_number(3).
