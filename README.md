# CARO DEV-C++

Phần này dành cho phiên bản game Caro viết bằng C và SDL2 trên Dev-C++.

## Thành viên phụ trách
- Ninh Tiến Đạt: giao diện SDL2, bàn cờ, hiển thị quân cờ và thao tác chuột.
- Lê Hồng Gia Thương: luật chơi, kiểm tra thắng/thua/hòa, chế độ hai người và AI.

## Các tệp cần đưa vào thư mục này
Hiện tại repository mới có file cấu hình `Project3.dev` được cung cấp. Cần bổ sung các tệp mã nguồn thật từ máy đã phát triển game:
- `caro.cpp`
- `supabase.c` và `supabase.h` (nếu phiên bản C++ đang sử dụng Supabase)
- `SDL2.dll` (để chạy trên máy Windows phù hợp)
- `Project3.exe` (không bắt buộc, nhưng có thể đính kèm để giảng viên chạy thử)

**Lưu ý:** Không thể biên dịch chỉ với `Project3.dev`. File cấu hình hiện chứa đường dẫn SDL2 trên máy phát triển (`C:\Users\LENOVO\Downloads\...`). Khi mở trên máy khác, cần chỉnh lại đường dẫn Include/Lib trong Dev-C++ cho đúng máy đó.
