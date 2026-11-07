/* =========================================
   1. KET NOI SUPABASE
========================================= */

const SUPABASE_URL = "https://ijczadawfwwgcrgcwion.supabase.co";
const SUPABASE_KEY = "sb_publishable_GFt7LYSIEAnS6LYhrtxeAg_KOy8iMpf";

const supabaseClient = window.supabase.createClient(
    SUPABASE_URL,
    SUPABASE_KEY
);


/* =========================================
   2. KIEM TRA TAI KHOAN DANG NHAP
========================================= */

let taiKhoanHienTai = null;

async function kiemTraDangNhap() {
    const { data, error } = await supabaseClient.auth.getUser();

    if (error || !data.user) {
        taiKhoanHienTai = null;
        await capNhatThongTinTaiKhoan();
        return;
    }

    taiKhoanHienTai = data.user;
    await capNhatThongTinTaiKhoan();
}


/* =========================================
   3. HIEN THI TEN TAI KHOAN
========================================= */

async function capNhatThongTinTaiKhoan() {
    const khuVuc = document.getElementById("thongTinTaiKhoan");

    if (!khuVuc) return;

    if (!taiKhoanHienTai) {
        khuVuc.textContent = "Bạn chưa đăng nhập";
        return;
    }

    const { data, error } = await supabaseClient
        .from("profiles")
        .select("username")
        .eq("id", taiKhoanHienTai.id)
        .maybeSingle();

    if (!error && data) {
        khuVuc.textContent = "Xin chào: " + data.username;
    } else {
        khuVuc.textContent = "Xin chào: " + taiKhoanHienTai.email;
    }
}


/* =========================================
   4. DANG KY TAI KHOAN
========================================= */

async function xuLyDangKy() {
    const tenDangNhap =
        document.getElementById("tenDangKy").value.trim();

    const email =
        document.getElementById("emailDangKy").value.trim();

    const matKhau =
        document.getElementById("matKhauDangKy").value;

    const thongBao =
        document.getElementById("thongBaoDangKy");

    if (!tenDangNhap || !email || !matKhau) {
        thongBao.textContent = "Vui lòng nhập đầy đủ thông tin!";
        return;
    }

    if (matKhau.length < 6) {
        thongBao.textContent = "Mật khẩu phải có ít nhất 6 ký tự!";
        return;
    }

    thongBao.textContent = "Đang đăng ký...";

    const { data, error } = await supabaseClient.auth.signUp({
        email: email,
        password: matKhau,
        options: {
            data: {
                username: tenDangNhap
            }
        }
    });

    if (error) {
        thongBao.textContent = "Lỗi: " + error.message;
        return;
    }

    if (data.user && data.session) {
        taiKhoanHienTai = data.user;

        const { error: loiHoSo } = await supabaseClient
            .from("profiles")
            .upsert({
                id: data.user.id,
                username: tenDangNhap
            }, {
                onConflict: "id"
            });

        if (loiHoSo) {
            thongBao.textContent =
                "Tài khoản đã tạo nhưng chưa tạo được hồ sơ: " +
                loiHoSo.message;
            return;
        }

        thongBao.textContent = "Đăng ký thành công!";
        await capNhatThongTinTaiKhoan();

    } else {
        thongBao.textContent =
            "Đã đăng ký. Hãy kiểm tra email để xác nhận tài khoản.";
    }
}


/* =========================================
   5. DANG NHAP
========================================= */

async function xuLyDangNhap() {
    const email =
        document.getElementById("emailDangNhap").value.trim();

    const matKhau =
        document.getElementById("matKhauDangNhap").value;

    const thongBao =
        document.getElementById("thongBaoDangNhap");

    if (!email || !matKhau) {
        thongBao.textContent = "Vui lòng nhập email và mật khẩu!";
        return;
    }

    thongBao.textContent = "Đang đăng nhập...";

    const { data, error } = await supabaseClient.auth
        .signInWithPassword({
            email: email,
            password: matKhau
        });

    if (error) {
        thongBao.textContent = "Lỗi: " + error.message;
        return;
    }

    taiKhoanHienTai = data.user;

    await capNhatThongTinTaiKhoan();

    thongBao.textContent = "Đăng nhập thành công!";
}


/* =========================================
   6. DANG XUAT
========================================= */

async function dangXuat() {
    const { error } = await supabaseClient.auth.signOut();

    if (error) {
        alert("Lỗi đăng xuất: " + error.message);
        return;
    }

    taiKhoanHienTai = null;

    await capNhatThongTinTaiKhoan();

    veMenu();
}


/* =========================================
   7. LAY BANG XEP HANG TOP 10
========================================= */

async function hienThiBangXepHang() {
    veManHinh("bangXepHang");

    const noiDung =
        document.getElementById("noiDungBangXepHang");

    noiDung.textContent = "Đang tải bảng xếp hạng...";

    const { data, error } = await supabaseClient
        .from("profiles")
        .select("username, rank_points, total_matches, total_wins")
        .order("rank_points", { ascending: false })
        .limit(10);

    if (error) {
        noiDung.textContent =
            "Không tải được bảng xếp hạng: " + error.message;
        return;
    }

    if (!data || data.length === 0) {
        noiDung.textContent = "Chưa có người chơi nào.";
        return;
    }

    const bang = document.createElement("table");
    const tieuDe = document.createElement("tr");

    ["Hạng", "Tên người chơi", "Điểm", "Số trận", "Số thắng"]
        .forEach(function(chu) {
            const o = document.createElement("th");
            o.textContent = chu;
            tieuDe.appendChild(o);
        });

    bang.appendChild(tieuDe);

    data.forEach(function(nguoiChoi, index) {
        const dong = document.createElement("tr");

        const giaTri = [
            index + 1,
            nguoiChoi.username,
            nguoiChoi.rank_points,
            nguoiChoi.total_matches,
            nguoiChoi.total_wins
        ];

        giaTri.forEach(function(chu) {
            const o = document.createElement("td");
            o.textContent = chu;
            dong.appendChild(o);
        });

        bang.appendChild(dong);
    });

    noiDung.innerHTML = "";
    noiDung.appendChild(bang);
}


/* =========================================
   8. TAO PHONG CHOI ONLINE
========================================= */

async function taoPhongSupabase() {
    if (!taiKhoanHienTai) {
        throw new Error("Bạn cần đăng nhập trước!");
    }

    const maPhong = Math.random()
        .toString(36)
        .substring(2, 8)
        .toUpperCase();

    const { data, error } = await supabaseClient
        .from("caro_rooms")
        .insert({
            room_code: maPhong,
            host_id: taiKhoanHienTai.id,
            board: [],
            current_turn: "X",
            status: "waiting"
        })
        .select()
        .single();

    if (error) throw error;

    return data;
}


/* =========================================
   9. THAM GIA PHONG ONLINE
========================================= */

async function thamGiaPhongSupabase(maPhong) {
    if (!taiKhoanHienTai) {
        throw new Error("Bạn cần đăng nhập trước!");
    }

    const { data, error } = await supabaseClient
        .rpc("join_caro_room", {
            p_room_code: maPhong.trim().toUpperCase()
        });

    if (error) throw error;

    return Array.isArray(data) ? data[0] : data;
}


/* =========================================
   10. LUU TRAN AI
========================================= */

async function luuTranAI(mode, result) {
    if (!taiKhoanHienTai) {
        throw new Error("Bạn chưa đăng nhập!");
    }

    const { data, error } = await supabaseClient.rpc(
        "luu_tran_ai",
        {
            p_mode: mode,
            p_result: result
        }
    );

    if (error) throw error;

    return data;
}


/* =========================================
   11. LAY LICH SU TRAN DAU
========================================= */

async function layLichSuTranDau() {
    if (!taiKhoanHienTai) {
        throw new Error("Bạn cần đăng nhập để xem lịch sử!");
    }

    const idNguoiChoi = taiKhoanHienTai.id;

    const { data, error } = await supabaseClient
        .from("matches")
        .select("id, player_x, player_o, winner_id, result, mode, played_at")
        .or(
            "player_x.eq." + idNguoiChoi +
            ",player_o.eq." + idNguoiChoi
        )
        .order("played_at", { ascending: false })
        .limit(50);

    if (error) throw error;

    return data || [];
}


/* =========================================
   12. KHOI TAO KIEM TRA DANG NHAP
========================================= */

document.addEventListener("DOMContentLoaded", function() {
    kiemTraDangNhap();
});
