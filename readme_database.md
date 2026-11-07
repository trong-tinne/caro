# 🗄️ DATABASE – GAME CARO

## 1. Giới thiệu

Dự án GAME CARO sử dụng **Supabase** làm nền tảng Backend để quản lý tài khoản người chơi, lưu trữ thông tin trận đấu, cập nhật điểm xếp hạng và hỗ trợ chức năng chơi trực tuyến.

Supabase sử dụng PostgreSQL làm hệ quản trị cơ sở dữ liệu, đồng thời cung cấp các chức năng xác thực người dùng và đồng bộ dữ liệu theo thời gian thực.

## 2. Công nghệ sử dụng

| Công nghệ | Chức năng |
|---|---|
| Supabase | Nền tảng Backend |
| PostgreSQL | Quản lý và lưu trữ dữ liệu |
| Supabase Auth | Đăng ký, đăng nhập và xác thực tài khoản |
| Supabase Realtime | Đồng bộ dữ liệu phòng chơi trực tuyến |
| RPC Functions | Xử lý các thao tác trò chơi thông qua hàm SQL |
| Triggers | Tự động cập nhật thống kê và điểm Rank |
| Row Level Security (RLS) | Phân quyền truy cập dữ liệu |

## 3. Cấu trúc cơ sở dữ liệu

Dự án sử dụng ba bảng chính:

### 3.1. Bảng `profiles`

Lưu trữ thông tin hồ sơ của người chơi.

Các thông tin chính:
- `id`: ID người chơi, liên kết với tài khoản Supabase Auth.
- `username`: Tên hiển thị của người chơi.
- `rank_points`: Điểm xếp hạng hiện tại.
- `total_matches`: Tổng số trận đã chơi.
- `total_wins`: Tổng số trận thắng.
- `total_losses`: Tổng số trận thua.
- `total_draws`: Tổng số trận hòa.
- `created_at`: Thời điểm tạo hồ sơ.

**Chức năng:**
- Quản lý thông tin người chơi.
- Lưu điểm Rank.
- Thống kê thành tích thi đấu.
- Cung cấp dữ liệu cho bảng xếp hạng.

### 3.2. Bảng `matches`

Lưu trữ lịch sử các trận đấu.

Các thông tin chính:
- `id`: Mã trận đấu.
- `player_x`: ID người chơi X.
- `player_o`: ID người chơi O (có thể NULL khi chơi với AI).
- `winner_id`: ID người chiến thắng (có thể NULL khi hòa hoặc AI thắng).
- `result`: Kết quả trận đấu.
- `mode`: Chế độ chơi.
- `played_at`: Thời điểm hoàn thành trận đấu.

Các chế độ chơi:
- `PVP`: Hai người chơi.
- `AI`: Chơi với máy tính.
- `AI_KHA`: Chơi với AI mức Khá.
- `AI_KHO`: Chơi với AI mức Khó.

**Chức năng:**
- Lưu kết quả từng trận.
- Hiển thị lịch sử thi đấu.
- Làm cơ sở để cập nhật điểm Rank và thống kê người chơi.

### 3.3. Bảng `caro_rooms`

Quản lý các phòng chơi Caro trực tuyến.

Các thông tin chính:
- `id`: ID phòng.
- `room_code`: Mã phòng để người khác tham gia.
- `host_id`: ID người tạo phòng.
- `guest_id`: ID người tham gia.
- `board`: Dữ liệu bàn cờ.
- `current_turn`: Lượt đánh hiện tại.
- `status`: Trạng thái phòng.
- `result`: Kết quả trận đấu.
- `winner_id`: ID người chiến thắng.
- `created_at`: Thời điểm tạo phòng.
- `updated_at`: Thời điểm cập nhật gần nhất.

Các trạng thái phòng:
- `waiting`: Đang chờ người chơi.
- `playing`: Đang diễn ra trận đấu.
- `finished`: Trận đấu đã kết thúc.

**Chức năng:**
- Tạo phòng chơi trực tuyến.
- Cho phép người khác tham gia bằng mã phòng.
- Lưu trạng thái bàn cờ và lượt đánh.
- Đồng bộ diễn biến trận đấu giữa hai người chơi.

## 4. Các RPC Functions

Dự án sử dụng ba hàm RPC chính để xử lý chức năng chơi trực tuyến và lưu kết quả.

| Tên hàm | Chức năng |
|---|---|
| `join_caro_room` | Cho phép người chơi tham gia phòng bằng mã phòng |
| `luu_tran_ai` | Lưu kết quả trận đấu với AI |
| `make_caro_move` | Xử lý nước đi của người chơi trong phòng trực tuyến |

### Chi tiết

**`join_caro_room(p_room_code)`**
- Kiểm tra người chơi đã đăng nhập.
- Tìm phòng theo mã phòng.
- Kiểm tra trạng thái phòng.
- Thêm người chơi thứ hai vào phòng.

**`luu_tran_ai(p_mode, p_result)`**
- Kiểm tra tài khoản người chơi.
- Kiểm tra chế độ chơi và kết quả.
- Lưu thông tin trận đấu với AI vào bảng `matches`.

**`make_caro_move(p_room_code, p_row, p_col)`**
- Kiểm tra người chơi có thuộc phòng hay không.
- Kiểm tra lượt đánh.
- Cập nhật vị trí X hoặc O trên bàn cờ.
- Kiểm tra thắng, thua hoặc hòa.
- Cập nhật trạng thái phòng và lưu kết quả trận đấu khi kết thúc.

## 5. Triggers và cơ chế cập nhật Rank

Dự án sử dụng Trigger để tự động cập nhật điểm Rank và thống kê sau khi trận đấu được lưu thành công.

Hàm xử lý chính:

`cap_nhat_rank_sau_tran()`

Trigger:

`trigger_cap_nhat_rank`

Được thực thi sau khi có bản ghi mới được thêm vào bảng `matches`.

Quy tắc cộng điểm:

| Kết quả | Thay đổi điểm Rank |
|---|---:|
| Thắng | +20 |
| Thua | -10 |
| Hòa | +5 |

Ngoài điểm Rank, hệ thống còn cập nhật:
- Tổng số trận đã chơi.
- Số trận thắng.
- Số trận thua.
- Số trận hòa.

## 6. Xác thực và bảo mật dữ liệu

Dự án sử dụng Supabase Auth để quản lý tài khoản và Row Level Security (RLS) để kiểm soát quyền truy cập.

Các nguyên tắc bảo mật:
- Người chơi cần đăng nhập để thực hiện các thao tác yêu cầu xác thực.
- Người chơi chỉ được cập nhật hồ sơ của chính mình theo chính sách RLS.
- Người chơi chỉ được xem lịch sử trận đấu có liên quan đến tài khoản của mình.
- Bảng xếp hạng cho phép người dùng xem thông tin công khai.
- Các thao tác phòng chơi được kiểm tra quyền tham gia.

**Lưu ý bảo mật:** Không đưa mật khẩu, `service_role key` hoặc thông tin bí mật vào GitHub. Phía trình duyệt chỉ sử dụng publishable key (hoặc anon key theo cấu hình cũ) kết hợp với RLS phù hợp.

## 7. Cấu trúc thư mục

```text
03_DATABASE/
│
├── README.md
└── database.sql
```

- `README.md`: Tài liệu mô tả cấu trúc và chức năng cơ sở dữ liệu.
- `database.sql`: Tập hợp các câu lệnh SQL dùng để tạo bảng, hàm, trigger, chính sách RLS và cấu hình liên quan.

**Lưu ý:** File `database.sql` cần chứa SQL thực tế đã được nhóm sử dụng và kiểm tra trên Supabase. Không sử dụng schema phỏng đoán thay cho cấu trúc đang hoạt động.

## 8. Kết luận

Cơ sở dữ liệu Supabase đóng vai trò quan trọng trong phiên bản GAME CARO WEB, giúp hệ thống quản lý tài khoản, lưu lịch sử trận đấu, cập nhật thành tích, tính điểm Rank và hỗ trợ chơi trực tuyến giữa hai người chơi.

Việc kết hợp PostgreSQL, RPC Functions, Triggers, Supabase Auth và Realtime giúp dự án có thể mở rộng và quản lý dữ liệu tập trung.

---

**Dự án:** GAME CARO – Phiên bản Web

**Nền tảng Backend:** Supabase

**Hệ quản trị cơ sở dữ liệu:** PostgreSQL