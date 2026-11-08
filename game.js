
/* =========================================
   1. KHAI BAO BIEN GAME
========================================= */

const SO_O = 15;

let banCo = [];
let nguoiDi = "X";
let cheDoChoi = "AI_KHA";
let ketThuc = false;
let daLuuTran = false;

let phongHienTai = null;
let dangChoPhong = false;


/* =========================================
   2. CAC HAM HIEN THI MAN HINH
========================================= */

function veManHinh(id) {
    const cacManHinh = [
        "menu",
        "dangNhap",
        "dangKy",
        "khuVucGame",
        "phongOnline",
        "bangXepHang",
        "lichSu"
    ];

    cacManHinh.forEach(function(ten) {
        const manHinh = document.getElementById(ten);
        if (manHinh) {
            manHinh.classList.add("an");
        }
    });

    const manHinhCanHien = document.getElementById(id);

    if (manHinhCanHien) {
        manHinhCanHien.classList.remove("an");
    }
}

function veMenu() {
    ngungRealtimePhong();

    phongHienTai = null;
    dangChoPhong = false;
    ketThuc = false;

    veManHinh("menu");
}
function hienThiDangNhap() {
    veManHinh("dangNhap");
}

function hienThiDangKy() {
    veManHinh("dangKy");
}


/* =========================================
   3. TAO BAN CO 15 X 15
========================================= */

function taoBanCo() {
    banCo = [];

    for (let i = 0; i < SO_O; i++) {
        banCo[i] = [];

        for (let j = 0; j < SO_O; j++) {
            banCo[i][j] = "";
        }
    }

    nguoiDi = "X";
    ketThuc = false;
    daLuuTran = false;
    dangChoPhong = false;
}


/* =========================================
   4. VE BAN CO LEN MAN HINH
========================================= */

function veBanCo() {
    const khuVuc = document.getElementById("banCo");

    if (!khuVuc) {
        console.error("Không tìm thấy khu vực banCo trong HTML!");
        return;
    }

    khuVuc.innerHTML = "";

    for (let i = 0; i < SO_O; i++) {
        for (let j = 0; j < SO_O; j++) {
            const o = document.createElement("div");

            o.className = "oCo";
            o.textContent = banCo[i][j];

            if (banCo[i][j] === "X") {
                o.style.color = "#dc2626";
            }

            if (banCo[i][j] === "O") {
                o.style.color = "#2563eb";
            }

            o.onclick = function() {
                xuLyDanhCo(i, j);
            };

            khuVuc.appendChild(o);
        }
    }

    const luotDi = document.getElementById("luotDi");

    if (luotDi) {
        luotDi.textContent = "Lượt đi: " + nguoiDi;
    }
}


/* =========================================
   5. BAT DAU GAME VOI AI
========================================= */

function batDauAI(mucDo) {
    if (!taiKhoanHienTai) {
        alert("Vui lòng đăng nhập trước khi chơi!");
        hienThiDangNhap();
        return;
    }

    if (mucDo !== "AI_KHA" && mucDo !== "AI_KHO") {
        alert("Chế độ AI không hợp lệ!");
        return;
    }

    cheDoChoi = mucDo;
    phongHienTai = null;

    taoBanCo();

    document.getElementById("tieuDeGame").textContent =
        mucDo === "AI_KHA"
            ? "CARO - AI KHÁ"
            : "CARO - AI KHÓ";

    document.getElementById("thongBaoGame").textContent =
        "Bạn là X. Hãy đi nước đầu tiên!";

    veManHinh("khuVucGame");
    veBanCo();
}


/* =========================================
   6. BAT DAU GAME 2 NGUOI ONLINE
========================================= */

function batDauHaiNguoi() {
    if (!taiKhoanHienTai) {
        alert("Vui lòng đăng nhập trước!");
        hienThiDangNhap();
        return;
    }

    veManHinh("phongOnline");
}


/* =========================================
   7. XU LY NGUOI CHOI DANH CO
========================================= */

function xuLyDanhCo(i, j) {
    if (ketThuc || dangChoPhong) {
        return;
    }

    if (banCo[i][j] !== "") {
        return;
    }

    // Neu dang choi online thi gui nuoc di len Supabase
if (phongHienTai) {
    danhCoOnline(i, j);
    return;
}
    // Người chơi X đánh
    banCo[i][j] = "X";

    veBanCo();

    // Kiểm tra X thắng
    if (kiemTraThang(i, j, "X")) {
        ketThucVaLuuTran("X_WIN");
        return;
    }

    // Kiểm tra hòa
    if (kiemTraHoa()) {
        ketThucVaLuuTran("DRAW");
        return;
    }

    // Chuyển lượt cho AI
    nguoiDi = "O";
    veBanCo();

    dangChoPhong = true;

    document.getElementById("thongBaoGame").textContent =
        "AI đang suy nghĩ...";

    setTimeout(function() {

        nuocDiAI();

        dangChoPhong = false;

        veBanCo();

        if (ketThuc) {
            return;
        }

        if (kiemTraHoa()) {
            ketThucVaLuuTran("DRAW");
            return;
        }

        // Chuyển lượt lại cho người chơi
        nguoiDi = "X";
        veBanCo();

        document.getElementById("thongBaoGame").textContent =
            "Đến lượt bạn!";

    }, 350);
}


/* =========================================
   8. KIEM TRA THANG THEO 4 HUONG
========================================= */

function demQuan(i, j, dx, dy, kyHieu) {
    let dem = 0;

    let x = i + dx;
    let y = j + dy;

    while (
        x >= 0 && x < SO_O &&
        y >= 0 && y < SO_O &&
        banCo[x][y] === kyHieu
    ) {
        dem++;
        x += dx;
        y += dy;
    }

    return dem;
}

function kiemTraThang(i, j, kyHieu) {
    const huong = [
        [0, 1],
        [1, 0],
        [1, 1],
        [1, -1]
    ];

    for (let k = 0; k < huong.length; k++) {
        const dx = huong[k][0];
        const dy = huong[k][1];

        const tong =
            1 +
            demQuan(i, j, dx, dy, kyHieu) +
            demQuan(i, j, -dx, -dy, kyHieu);

        if (tong >= 5) {
            return true;
        }
    }

    return false;
}


/* =========================================
   9. KIEM TRA HOA
========================================= */

function kiemTraHoa() {
    for (let i = 0; i < SO_O; i++) {
        for (let j = 0; j < SO_O; j++) {
            if (banCo[i][j] === "") {
                return false;
            }
        }
    }

    return true;
}


/* =========================================
   10. AI TIM NUOC DI
========================================= */

/* Tim nuoc thang ngay cho X hoac O */

function timNuocThang(kyHieu) {
    for (let i = 0; i < SO_O; i++) {
        for (let j = 0; j < SO_O; j++) {

            if (banCo[i][j] === "") {

                banCo[i][j] = kyHieu;

                if (kiemTraThang(i, j, kyHieu)) {
                    banCo[i][j] = "";
                    return [i, j];
                }

                banCo[i][j] = "";
            }
        }
    }

    return null;
}


/* Tinh diem mot huong cua mot quan co */

function diemMotHuong(i, j, dx, dy, kyHieu) {
    let dem = 1;
    let dauMo = 0;

    // Dem theo chieu duong
    let x = i + dx;
    let y = j + dy;

    while (
        x >= 0 && x < SO_O &&
        y >= 0 && y < SO_O &&
        banCo[x][y] === kyHieu
    ) {
        dem++;
        x += dx;
        y += dy;
    }

    if (
        x >= 0 && x < SO_O &&
        y >= 0 && y < SO_O &&
        banCo[x][y] === ""
    ) {
        dauMo++;
    }

    // Dem theo chieu am
    x = i - dx;
    y = j - dy;

    while (
        x >= 0 && x < SO_O &&
        y >= 0 && y < SO_O &&
        banCo[x][y] === kyHieu
    ) {
        dem++;
        x -= dx;
        y -= dy;
    }

    if (
        x >= 0 && x < SO_O &&
        y >= 0 && y < SO_O &&
        banCo[x][y] === ""
    ) {
        dauMo++;
    }

    // Bang diem
    if (dem >= 5) return 100000;

    if (dem === 4 && dauMo === 2) return 10000;
    if (dem === 4 && dauMo === 1) return 5000;

    if (dem === 3 && dauMo === 2) return 1000;
    if (dem === 3 && dauMo === 1) return 200;

    if (dem === 2 && dauMo === 2) return 100;
    if (dem === 2 && dauMo === 1) return 20;

    return dem;
}


/* Tinh tong diem cua mot o theo 4 huong */

function tinhDiem(i, j, kyHieu) {
    let diem = 0;

    diem += diemMotHuong(i, j, 0, 1, kyHieu);
    diem += diemMotHuong(i, j, 1, 0, kyHieu);
    diem += diemMotHuong(i, j, 1, 1, kyHieu);
    diem += diemMotHuong(i, j, 1, -1, kyHieu);

    return diem;
}


/* AI chon nuoc di theo muc do */

function timNuocAI() {

    // Buoc 1: O co nuoc thang thi danh ngay
    let nuocThang = timNuocThang("O");

    if (nuocThang !== null) {
        return nuocThang;
    }

    // Buoc 2: Chan nuoc thang cua X
    let nuocChan = timNuocThang("X");

    if (nuocChan !== null) {
        return nuocChan;
    }

    // Buoc 3: Tinh diem cac o trong
    let danhSach = [];

    for (let i = 0; i < SO_O; i++) {
        for (let j = 0; j < SO_O; j++) {

            if (banCo[i][j] !== "") {
                continue;
            }

            let diem = 0;

            // Tinh diem tan cong O
            banCo[i][j] = "O";

            diem = tinhDiem(i, j, "O");

            banCo[i][j] = "";

            // Tinh diem phong thu X
            banCo[i][j] = "X";

            diem += tinhDiem(i, j, "X") * 2;

            banCo[i][j] = "";

            // Uu tien o gan trung tam
            diem += SO_O -
                Math.floor(
                    (
                        Math.abs(i - Math.floor(SO_O / 2)) +
                        Math.abs(j - Math.floor(SO_O / 2))
                    ) / 2
                );

            danhSach.push({
                dong: i,
                cot: j,
                diem: diem
            });
        }
    }

    // Sap xep diem tu cao xuong thap
    danhSach.sort(function(a, b) {
        return b.diem - a.diem;
    });

    if (danhSach.length === 0) {
        return null;
    }

    // AI KHA: chon ngau nhien trong 3 o tot nhat
    if (cheDoChoi === "AI_KHA") {

        let soLuaChon = Math.min(3, danhSach.length);

        let viTri = Math.floor(Math.random() * soLuaChon);

        return [
            danhSach[viTri].dong,
            danhSach[viTri].cot
        ];
    }

    // AI KHO: chon o co diem cao nhat
    return [
        danhSach[0].dong,
        danhSach[0].cot
    ];
}


/* Thuc hien nuoc di cua AI */

function nuocDiAI() {

    let nuocDi = timNuocAI();

    if (nuocDi === null) {
        return;
    }

    let i = nuocDi[0];
    let j = nuocDi[1];

    banCo[i][j] = "O";

    if (kiemTraThang(i, j, "O")) {
        ketThucVaLuuTran("O_WIN");
    }
}



/* =========================================
   11. BAT DAU PHONG ONLINE
========================================= */

function batDauPhongOnline(phong) {
    phongHienTai = phong;
    cheDoChoi = "PVP";
    daThongBaoKetThucOnline = false;

    taoBanCo();

    phongHienTai = phong;

    document.getElementById("tieuDeGame").textContent =
        "CARO - HAI NGUOI ONLINE";

    const thongBao = document.getElementById("thongBaoGame");

    if (phong.host_id === taiKhoanHienTai.id) {
        thongBao.textContent =
            phong.status === "waiting"
                ? "Bạn là X. Hãy gửi mã phòng cho đối thủ!"
                : "Bạn là X. Đến lượt bạn khi bàn cờ hiển thị X.";
    } else {
        thongBao.textContent =
            "Bạn là O. Đang vào phòng...";
    }

    veManHinh("khuVucGame");

    langNghePhongOnline(phong);
    capNhatPhongTuServer(phong);
}


/* =========================================
   12. TAO PHONG ONLINE
========================================= */

async function taoPhong() {
    const thongBao = document.getElementById("thongBaoPhong");

    thongBao.textContent = "Đang tạo phòng...";

    try {
        const phong = await taoPhongSupabase();

        batDauPhongOnline(phong);

        thongBao.textContent =
            "Mã phòng của bạn: " + phong.room_code;

        alert(
            "Tạo phòng thành công!\nMã phòng: " +
            phong.room_code +
            "\nHãy gửi mã này cho bạn chơi cùng."
        );

    } catch (error) {
        thongBao.textContent = "Lỗi: " + error.message;
    }
}


/* =========================================
   13. THAM GIA PHONG ONLINE
========================================= */

async function thamGiaPhong() {
    const maPhong =
        document.getElementById("maPhong").value.trim();

    const thongBao =
        document.getElementById("thongBaoPhong");

    if (!maPhong) {
        thongBao.textContent = "Vui lòng nhập mã phòng!";
        return;
    }

    thongBao.textContent = "Đang tham gia phòng...";

    try {
        const phong = await thamGiaPhongSupabase(maPhong);

        batDauPhongOnline(phong);

        thongBao.textContent = "Tham gia phòng thành công!";

    } catch (error) {
        thongBao.textContent = "Lỗi: " + error.message;
    }
}


/* =========================================
   13. HIEN THI LICH SU TRAN DAU
========================================= */

async function hienThiLichSu() {
    veManHinh("lichSu");

    const noiDung = document.getElementById("noiDungLichSu");

    if (!taiKhoanHienTai) {
        noiDung.textContent =
            "Vui lòng đăng nhập để xem lịch sử trận đấu!";
        return;
    }

    noiDung.textContent = "Đang tải lịch sử trận đấu...";

    try {
        const danhSach = await layLichSuTranDau();

        if (danhSach.length === 0) {
            noiDung.textContent = "Bạn chưa có trận đấu nào.";
            return;
        }

        const bang = document.createElement("table");

        const dongTieuDe = document.createElement("tr");

        ["Mã trận", "Chế độ", "Kết quả", "Thời gian"]
            .forEach(function(tieuDe) {
                const o = document.createElement("th");
                o.textContent = tieuDe;
                dongTieuDe.appendChild(o);
            });

        bang.appendChild(dongTieuDe);

        danhSach.forEach(function(tran) {
            const dong = document.createElement("tr");

            // Xac dinh che do
            let tenCheDo = tran.mode;

            if (tran.mode === "AI_KHA") {
                tenCheDo = "AI Khá";
            } else if (tran.mode === "AI_KHO") {
                tenCheDo = "AI Khó";
            } else if (tran.mode === "AI") {
                tenCheDo = "AI";
            } else if (tran.mode === "PVP") {
                tenCheDo = "Hai người";
            }

            // Xac dinh ket qua
            let ketQua = "Không xác định";

            if (tran.result === "DRAW") {
                ketQua = "Hòa";
            } else if (
                tran.mode === "AI" ||
                tran.mode === "AI_KHA" ||
                tran.mode === "AI_KHO"
            ) {
                if (tran.result === "X_WIN") {
                    ketQua = "Thắng";
                } else if (tran.result === "O_WIN") {
                    ketQua = "Thua";
                }
            } else if (tran.mode === "PVP") {
                if (tran.winner_id === taiKhoanHienTai.id) {
                    ketQua = "Thắng";
                } else if (tran.winner_id) {
                    ketQua = "Thua";
                }
            }

            const thoiGian = tran.played_at
                ? new Date(tran.played_at).toLocaleString("vi-VN")
                : "Không rõ";

            const giaTri = [
                "#" + tran.id,
                tenCheDo,
                ketQua,
                thoiGian
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

    } catch (error) {
        console.error("Lỗi tải lịch sử:", error);

        noiDung.textContent =
            "Không tải được lịch sử trận đấu: " + error.message;
    }
}


/* =========================================
   14. KHOI TAO GAME
========================================= */

taoBanCo();

/* =========================================
   13. DONG BO PHONG CARO REALTIME
========================================= */

let kenhPhongOnline = null;
let daThongBaoKetThucOnline = false;

// Cap nhat ban co khi Supabase bao co thay doi
function capNhatPhongTuServer(phong) {
    if (!phong || !phongHienTai) return;
    if (phong.id !== phongHienTai.id) return;

    phongHienTai = phong;

    if (Array.isArray(phong.board) &&
        phong.board.length === 15) {
        banCo = phong.board;
    }

    nguoiDi = phong.current_turn || "X";
    ketThuc = phong.status === "finished";

    if (typeof veBanCo === "function") {
        veBanCo();
    }

    const thongBao = document.getElementById("thongBaoGame");

    if (thongBao) {
        if (phong.status === "waiting") {
            thongBao.textContent = "Đang chờ người chơi thứ hai...";
        } else if (phong.status === "playing") {
            const laLuotCuaMinh =
                (taiKhoanHienTai.id === phong.host_id && nguoiDi === "X") ||
                (taiKhoanHienTai.id === phong.guest_id && nguoiDi === "O");

            thongBao.textContent = laLuotCuaMinh
                ? "Đến lượt bạn!"
                : "Đang chờ đối thủ đánh...";
        } else if (phong.status === "finished") {
            if (phong.result === "DRAW") {
                thongBao.textContent = "Trận đấu hòa!";
            } else if (phong.winner_id === taiKhoanHienTai.id) {
                thongBao.textContent = "Bạn đã chiến thắng!";
            } else {
                thongBao.textContent = "Bạn đã thua!";
            }

            if (!daThongBaoKetThucOnline) {
                daThongBaoKetThucOnline = true;
                alert(thongBao.textContent);
            }
        }
    }
}


// Lang nghe thay doi cua mot phong
function langNghePhongOnline(phong) {
    if (kenhPhongOnline) {
        supabaseClient.removeChannel(kenhPhongOnline);
        kenhPhongOnline = null;
    }

    kenhPhongOnline = supabaseClient
        .channel("caro-room-" + phong.id)
        .on(
            "postgres_changes",
            {
                event: "UPDATE",
                schema: "public",
                table: "caro_rooms",
                filter: "id=eq." + phong.id
            },
            function(payload) {
                capNhatPhongTuServer(payload.new);
            }
        )
        .subscribe(function(status) {
            console.log("Realtime room:", status);
        });
}


// Gui nuoc di len Supabase
async function guiNuocDiOnline(dong, cot) {
    if (!phongHienTai || !taiKhoanHienTai) {
        throw new Error("Bạn chưa ở trong phòng online!");
    }

    const { data, error } = await supabaseClient.rpc(
        "make_caro_move",
        {
            p_room_code: phongHienTai.room_code,
            p_row: dong,
            p_col: cot
        }
    );

    if (error) throw error;

    const phongMoi = Array.isArray(data) ? data[0] : data;

    capNhatPhongTuServer(phongMoi);

    return phongMoi;
}


// Roi phong va ngung lang nghe
function ngungRealtimePhong() {
    if (kenhPhongOnline) {
        supabaseClient.removeChannel(kenhPhongOnline);
        kenhPhongOnline = null;
    }
}


/* =========================================
   15. KET THUC VA LUU TRAN DAU
========================================= */

async function ketThucVaLuuTran(ketQua) {

    if (daLuuTran) {
        return;
    }

    daLuuTran = true;
    ketThuc = true;

    document.getElementById("thongBaoGame").textContent =
        "Đang lưu kết quả trận đấu...";

    try {
        const idTran = await luuTranAI(cheDoChoi, ketQua);

        let thongBao = "";

        if (ketQua === "X_WIN") {
            thongBao = "Bạn thắng! +20 điểm.";
        } else if (ketQua === "O_WIN") {
            thongBao = "AI thắng! Bạn bị trừ 10 điểm.";
        } else {
            thongBao = "Trận đấu hòa! +5 điểm.";
        }

        document.getElementById("thongBaoGame").textContent =
            thongBao + " Đã lưu trận #" + idTran;

        await capNhatThongTinTaiKhoan();

        alert(thongBao + "\nLưu trận đấu thành công!");

    } catch (error) {

        daLuuTran = false;

        console.error("CHI TIET LOI LUU TRAN:", error);

        let noiDungLoi = error.message || JSON.stringify(error);

        document.getElementById("thongBaoGame").textContent =
            "Không lưu được trận đấu: " + noiDungLoi;

        alert("CHI TIET LOI LUU TRAN:\n" + noiDungLoi);
    }
}

/* =========================================
   16. XU LY DANH CO ONLINE
========================================= */

async function danhCoOnline(i, j) {
    if (!phongHienTai || ketThuc || dangChoPhong) {
        return;
    }

    // Kiem tra nguoi choi co dung luot khong
    const kyHieuCuaMinh =
        taiKhoanHienTai.id === phongHienTai.host_id
            ? "X"
            : "O";

    if (nguoiDi !== kyHieuCuaMinh) {
        alert("Chưa đến lượt của bạn!");
        return;
    }

    if (banCo[i][j] !== "") {
        return;
    }

    dangChoPhong = true;

    try {
        await guiNuocDiOnline(i, j);
    } catch (error) {
        alert("Không đánh được: " + error.message);
    } finally {
        dangChoPhong = false;
    }
}

