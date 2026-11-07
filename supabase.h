
#ifndef SUPABASE_H
#define SUPABASE_H

#include <windows.h>
#include <winhttp.h>

/* Thong tin ket noi Supabase */
#define SUPABASE_URL L"https://ijczadawfwwgcrgcwion.supabase.co"
#define SUPABASE_KEY L"sb_publishable_GFt7LYSIEAnS6LYhrtxeAg_KOy8iMpf"

/* So nguoi choi hien thi tren bang xep hang */
#define SO_HANG_XEP_HANG 10

/* Thong tin mot nguoi choi trong bang xep hang */
typedef struct {
    char username[100];
    int rank_points;
    int total_matches;
    int total_wins;
} NguoiChoiXepHang;

#ifdef __cplusplus
extern "C" {
#endif

int kiemTraSupabase(void);

int dangKyTaiKhoan(
    const char *email,
    const char *matKhau,
    const char *tenDangNhap,
    char *phanHoi,
    DWORD kichThuocPhanHoi
);

int dangNhapTaiKhoan(
    const char *email,
    const char *matKhau,
    char *phanHoi,
    DWORD kichThuocPhanHoi
);

int dangNhapNguoiChoi(
    int viTriNguoiChoi,
    const char *email,
    const char *matKhau,
    char *phanHoi,
    DWORD kichThuocPhanHoi
);

const char *layIDNguoiChoi(int viTriNguoiChoi);
const char *layTenNguoiChoi(int viTriNguoiChoi);
int nguoiChoiDaDangNhap(int viTriNguoiChoi);
void dangXuatNguoiChoi(int viTriNguoiChoi);

int ghiNhanTranDau(
    const char *ketQua,
    const char *cheDo,
    char *phanHoi,
    DWORD kichThuocPhanHoi
);

/* Lay du lieu bang xep hang tu Supabase */
int layBangXepHang(
    NguoiChoiXepHang danhSach[],
    int soLuongToiDa,
    int *soLuongThucTe,
    char *phanHoi,
    DWORD kichThuocPhanHoi
);

#ifdef __cplusplus
}
#endif

#endif

