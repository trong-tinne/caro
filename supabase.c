#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winhttp.h>
#include <ctype.h>

#include "supabase.h"

#define SO_NGUOI_CHOI 2
#define DO_DAI_TOKEN 4096
#define DO_DAI_ID 100
#define DO_DAI_TEN 256

static char accessToken[SO_NGUOI_CHOI][DO_DAI_TOKEN];
static char idNguoiChoi[SO_NGUOI_CHOI][DO_DAI_ID];
static char tenNguoiChoi[SO_NGUOI_CHOI][DO_DAI_TEN];

static int layChuoiJSON(const char *json, const char *tenTruong,
                        char *ketQua, size_t kichThuoc) {
    char khoa[100];
    const char *p;
    size_t i = 0;

    if (!json || !tenTruong || !ketQua || kichThuoc == 0) return 0;
    snprintf(khoa, sizeof(khoa), "\"%s\"", tenTruong);
    p = strstr(json, khoa);
    if (!p) return 0;
    p += strlen(khoa);
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p != ':') return 0;
    p++;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p != '"') return 0;
    p++;

    while (*p && *p != '"') {
        char c = *p++;
        if (c == '\\') {
            if (*p == '\0') return 0;
            c = *p++;
            if (c == 'n') c = '\n';
            else if (c == 'r') c = '\r';
            else if (c == 't') c = '\t';
        }
        if (i + 1 >= kichThuoc) return 0;
        ketQua[i++] = c;
    }
    if (*p != '"') return 0;
    ketQua[i] = '\0';
    return 1;
}

static int guiYeuCauSupabase(const wchar_t *phuongThuc,
    const wchar_t *duongDan, const char *duLieuGui, const char *token,
    char *phanHoi, DWORD kichThuocPhanHoi, DWORD *maTrangThai) {

    HINTERNET ketNoi = NULL, mayChu = NULL, yeuCau = NULL;
    URL_COMPONENTS url;
    wchar_t tenMay[256], headerAPI[512], headerAuth[8192], tokenW[DO_DAI_TOKEN];
    DWORD trangThai = 0, kichThuocMa = sizeof(trangThai);
    DWORD conLai = 0, daDoc = 0, tongDaDoc = 0;
    int ketQua = 0;

    if (phanHoi && kichThuocPhanHoi > 0) phanHoi[0] = '\0';
    if (maTrangThai) *maTrangThai = 0;

    ZeroMemory(&url, sizeof(url));
    ZeroMemory(tenMay, sizeof(tenMay));
    url.dwStructSize = sizeof(url);
    url.lpszHostName = tenMay;
    url.dwHostNameLength = 256;

    if (!WinHttpCrackUrl(SUPABASE_URL, 0, 0, &url)) goto ketThuc;

    ketNoi = WinHttpOpen(L"GameCaro", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                         WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!ketNoi) goto ketThuc;

    mayChu = WinHttpConnect(ketNoi, url.lpszHostName, url.nPort, 0);
    if (!mayChu) goto ketThuc;

    yeuCau = WinHttpOpenRequest(mayChu, phuongThuc, duongDan, NULL,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        url.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);
    if (!yeuCau) goto ketThuc;

    wsprintfW(headerAPI, L"apikey: %s\r\n", SUPABASE_KEY);
    if (!WinHttpAddRequestHeaders(yeuCau, headerAPI, (DWORD)-1,
                                  WINHTTP_ADDREQ_FLAG_ADD)) goto ketThuc;

    if (token && token[0]) {
        if (!MultiByteToWideChar(CP_UTF8, 0, token, -1, tokenW, DO_DAI_TOKEN))
        goto ketThuc;
        wsprintfW(headerAuth, L"Authorization: Bearer %s\r\n", tokenW);
        if (!WinHttpAddRequestHeaders(yeuCau, headerAuth, (DWORD)-1,
                                      WINHTTP_ADDREQ_FLAG_ADD)) goto ketThuc;
    }

    if (duLieuGui != NULL) {
        if (!WinHttpAddRequestHeaders(yeuCau,
            L"Content-Type: application/json\r\n", (DWORD)-1,
            WINHTTP_ADDREQ_FLAG_ADD)) goto ketThuc;

        {
            DWORD kichThuocDuLieu = (DWORD)strlen(duLieuGui);
            if (!WinHttpSendRequest(yeuCau, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                (LPVOID)duLieuGui, kichThuocDuLieu, kichThuocDuLieu, 0))
                goto ketThuc;
        }
    } else {
        if (!WinHttpSendRequest(yeuCau, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) goto ketThuc;
    }

    if (!WinHttpReceiveResponse(yeuCau, NULL)) goto ketThuc;

    if (!WinHttpQueryHeaders(yeuCau,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &trangThai, &kichThuocMa,
        WINHTTP_NO_HEADER_INDEX)) goto ketThuc;

    if (maTrangThai) *maTrangThai = trangThai;

    if (phanHoi && kichThuocPhanHoi > 1) {
        while (tongDaDoc < kichThuocPhanHoi - 1) {
            conLai = 0;
            if (!WinHttpQueryDataAvailable(yeuCau, &conLai) || conLai == 0)
                break;
            if (conLai > kichThuocPhanHoi - 1 - tongDaDoc)
                conLai = kichThuocPhanHoi - 1 - tongDaDoc;
            daDoc = 0;
            if (!WinHttpReadData(yeuCau, phanHoi + tongDaDoc, conLai, &daDoc)
                || daDoc == 0) break;
            tongDaDoc += daDoc;
        }
        phanHoi[tongDaDoc] = '\0';
    }

    if (trangThai >= 200 && trangThai < 300) ketQua = 1;

ketThuc:
    if (yeuCau) WinHttpCloseHandle(yeuCau);
    if (mayChu) WinHttpCloseHandle(mayChu);
    if (ketNoi) WinHttpCloseHandle(ketNoi);
    return ketQua;
}

int kiemTraSupabase(void) {
    char phanHoi[2048];
    DWORD maTrangThai = 0;
    return guiYeuCauSupabase(L"GET",
        L"/rest/v1/profiles?select=id&limit=1", NULL, NULL,
        phanHoi, sizeof(phanHoi), &maTrangThai);
}

static int chuyenChuoiJSON(const char *nguon, char *dich, size_t kichThuoc) {
    size_t i, j = 0;
    if (!nguon || !dich || kichThuoc == 0) return 0;
    for (i = 0; nguon[i] != '\0'; i++) {
        unsigned char c = (unsigned char)nguon[i];
        if (c == '"' || c == '\\') {
            if (j + 2 >= kichThuoc) return 0;
            dich[j++] = '\\';
            dich[j++] = (char)c;
        } else {
            if (c < 32 || j + 1 >= kichThuoc) return 0;
            dich[j++] = (char)c;
        }
    }
    dich[j] = '\0';
    return 1;
}

int dangKyTaiKhoan(const char *email, const char *matKhau,
    const char *tenDangNhap, char *phanHoi, DWORD kichThuocPhanHoi) {

    char emailJSON[512], matKhauJSON[512], tenJSON[512], duLieuJSON[2048];
    DWORD maTrangThai = 0;
    int doDai, ketQua;
    if (!email || !matKhau || !tenDangNhap || !phanHoi ||
        kichThuocPhanHoi == 0 || !email[0] || !matKhau[0] || !tenDangNhap[0])
        return 0;

    if (!chuyenChuoiJSON(email, emailJSON, sizeof(emailJSON)) ||
        !chuyenChuoiJSON(matKhau, matKhauJSON, sizeof(matKhauJSON)) ||
        !chuyenChuoiJSON(tenDangNhap, tenJSON, sizeof(tenJSON))) return 0;

    doDai = snprintf(duLieuJSON, sizeof(duLieuJSON),
        "{\"email\":\"%s\",\"password\":\"%s\",\"data\":{\"username\":\"%s\"}}",
        emailJSON, matKhauJSON, tenJSON);
    if (doDai < 0 || (size_t)doDai >= sizeof(duLieuJSON)) return 0;

    ketQua = guiYeuCauSupabase(L"POST", L"/auth/v1/signup",
        duLieuJSON, NULL, phanHoi, kichThuocPhanHoi, &maTrangThai);
    printf("Ma HTTP dang ky: %lu\n", (unsigned long)maTrangThai);
    if (!ketQua) printf("Dang ky that bai: %s\n", phanHoi);
    return ketQua;
}

int dangNhapNguoiChoi(int viTriNguoiChoi, const char *email,
    const char *matKhau, char *phanHoi, DWORD kichThuocPhanHoi) {

    int viTri, doDai, ketQua;
    char emailJSON[512], matKhauJSON[512], duLieuJSON[1200];
    char tokenMoi[DO_DAI_TOKEN], idMoi[DO_DAI_ID], tenMoi[DO_DAI_TEN];
    DWORD maTrangThai = 0;

    if (viTriNguoiChoi < 1 || viTriNguoiChoi > 2 ||
        !email || !matKhau || !phanHoi || kichThuocPhanHoi == 0 ||
        !email[0] || !matKhau[0]) return 0;

    viTri = viTriNguoiChoi - 1;
    accessToken[viTri][0] = '\0';
    idNguoiChoi[viTri][0] = '\0';
    tenNguoiChoi[viTri][0] = '\0';

    if (!chuyenChuoiJSON(email, emailJSON, sizeof(emailJSON)) ||
        !chuyenChuoiJSON(matKhau, matKhauJSON, sizeof(matKhauJSON))) return 0;

    doDai = snprintf(duLieuJSON, sizeof(duLieuJSON),
        "{\"email\":\"%s\",\"password\":\"%s\"}", emailJSON, matKhauJSON);
    if (doDai < 0 || (size_t)doDai >= sizeof(duLieuJSON)) return 0;

    ketQua = guiYeuCauSupabase(L"POST",
        L"/auth/v1/token?grant_type=password", duLieuJSON, NULL,
        phanHoi, kichThuocPhanHoi, &maTrangThai);

    printf("Ma HTTP dang nhap: %lu\n", (unsigned long)maTrangThai);
    if (!ketQua) {
        printf("Dang nhap that bai: %s\n", phanHoi);
        return 0;
    }

    if (!layChuoiJSON(phanHoi, "access_token", tokenMoi, sizeof(tokenMoi)) ||
        !layChuoiJSON(phanHoi, "id", idMoi, sizeof(idMoi))) {
        printf("Khong lay duoc token hoac ID tu phan hoi.\n");
        return 0;
    }

    if (!layChuoiJSON(phanHoi, "username", tenMoi, sizeof(tenMoi))) {
        snprintf(tenMoi, sizeof(tenMoi), "Nguoi choi %c",
                 viTriNguoiChoi == 1 ? 'X' : 'O');
    }

    strcpy(accessToken[viTri], tokenMoi);
    strcpy(idNguoiChoi[viTri], idMoi);
    strcpy(tenNguoiChoi[viTri], tenMoi);

    printf("Nguoi choi %c dang nhap thanh cong. ID: %s; Ten: %s\n",
           viTriNguoiChoi == 1 ? 'X' : 'O',
           idNguoiChoi[viTri], tenNguoiChoi[viTri]);
    return 1;
}

int dangNhapTaiKhoan(const char *email, const char *matKhau,
    char *phanHoi, DWORD kichThuocPhanHoi) {
    	return dangNhapNguoiChoi(1, email, matKhau, phanHoi, kichThuocPhanHoi);
}

const char *layIDNguoiChoi(int viTriNguoiChoi) {
    if (viTriNguoiChoi < 1 || viTriNguoiChoi > 2) return "";
    return idNguoiChoi[viTriNguoiChoi - 1];
}

const char *layTenNguoiChoi(int viTriNguoiChoi) {
    if (viTriNguoiChoi < 1 || viTriNguoiChoi > 2) return "";
    return tenNguoiChoi[viTriNguoiChoi - 1];
}

int nguoiChoiDaDangNhap(int viTriNguoiChoi) {
    if (viTriNguoiChoi < 1 || viTriNguoiChoi > 2) return 0;
    return idNguoiChoi[viTriNguoiChoi - 1][0] != '\0' &&
           accessToken[viTriNguoiChoi - 1][0] != '\0';
}

void dangXuatNguoiChoi(int viTriNguoiChoi) {
    int viTri;
    if (viTriNguoiChoi < 1 || viTriNguoiChoi > 2) return;
    viTri = viTriNguoiChoi - 1;
    accessToken[viTri][0] = '\0';
    idNguoiChoi[viTri][0] = '\0';
    tenNguoiChoi[viTri][0] = '\0';
}

/* =====================================================
   LUU TRAN DAU:
   - 2_PLAYER: can ca X va O, ghi ca hai UUID.
   - AI_KHA / AI_KHO: X la nguoi, O trong database = NULL.
     Ket qua O_WIN co nghia AI thang, chi tru diem X.
   ===================================================== */
int ghiNhanTranDau(const char *ketQua, const char *cheDo,
    char *phanHoi, DWORD kichThuocPhanHoi) {

    char duLieuJSON[2000];
    DWORD maTrangThai = 0;
    const char *idX, *idO = NULL, *idNguoiThang = NULL;
    int laTranAI;
    int ketQuaGui; 

    if (!phanHoi || kichThuocPhanHoi == 0) return 0;
    phanHoi[0] = '\0';

    if (!ketQua || !cheDo) {
        snprintf(phanHoi, kichThuocPhanHoi, "Thieu ket qua hoac che do tran dau.");
        return 0;
    }

    laTranAI = (strcmp(cheDo, "AI_KHA") == 0 ||
                strcmp(cheDo, "AI_KHO") == 0);

    if (!laTranAI && strcmp(cheDo, "2_PLAYER") != 0) {
        snprintf(phanHoi, kichThuocPhanHoi, "Che do tran dau khong hop le.");
        return 0;
    }

    if (!nguoiChoiDaDangNhap(1)) {
        snprintf(phanHoi, kichThuocPhanHoi, "Nguoi choi X chua dang nhap.");
        return 0;
    }

    if (!laTranAI && !nguoiChoiDaDangNhap(2)) {
        snprintf(phanHoi, kichThuocPhanHoi, "Nguoi choi O chua dang nhap.");
        return 0;
    }

    idX = layIDNguoiChoi(1);
    if (!laTranAI) idO = layIDNguoiChoi(2);

    if (strcmp(ketQua, "X_WIN") == 0) {
        idNguoiThang = idX;
    } else if (strcmp(ketQua, "O_WIN") == 0) {
        if (!laTranAI) idNguoiThang = idO;
        /* Tran AI: AI khong co UUID, winner_id de NULL */
    } else if (strcmp(ketQua, "DRAW") != 0) {
        snprintf(phanHoi, kichThuocPhanHoi, "Ket qua tran dau khong hop le.");
        return 0;
    }

    if (laTranAI) {
        if (idNguoiThang != NULL) {
            snprintf(duLieuJSON, sizeof(duLieuJSON),
                "{\"player_x\":\"%s\",\"player_o\":null,"
                "\"winner_id\":\"%s\",\"result\":\"%s\",\"mode\":\"%s\"}",
                idX, idNguoiThang, ketQua, cheDo);
        } else {
            snprintf(duLieuJSON, sizeof(duLieuJSON),
            "{\"player_x\":\"%s\",\"player_o\":null,"
                "\"winner_id\":null,\"result\":\"%s\",\"mode\":\"%s\"}",
                idX, ketQua, cheDo);
        }
    } else {
        if (idNguoiThang != NULL) {
            snprintf(duLieuJSON, sizeof(duLieuJSON),
                "{\"player_x\":\"%s\",\"player_o\":\"%s\","
                "\"winner_id\":\"%s\",\"result\":\"%s\",\"mode\":\"%s\"}",
                idX, idO, idNguoiThang, ketQua, cheDo);
        } else {
            snprintf(duLieuJSON, sizeof(duLieuJSON),
                "{\"player_x\":\"%s\",\"player_o\":\"%s\","
                "\"winner_id\":null,\"result\":\"DRAW\",\"mode\":\"%s\"}",
                idX, idO, cheDo);
        }
    }

    ketQuaGui = guiYeuCauSupabase(
    L"POST",
    L"/rest/v1/matches",
    duLieuJSON,
    accessToken[0],
    phanHoi,
    kichThuocPhanHoi,
    &maTrangThai
);

printf("\n========== KIEM TRA LUU TRAN ==========\n");
printf("HTTP Status: %lu\n", (unsigned long)maTrangThai);
printf("Du lieu gui: %s\n", duLieuJSON);
printf("Phan hoi Supabase: %s\n", phanHoi);

return ketQuaGui;
}

/* =====================================================
   LAY BANG XEP HANG TOP 10 TU SUPABASE
   ===================================================== */

/* Doc gia tri so nguyen trong JSON */
static int laySoJSON(const char *json, const char *tenTruong) {
    char khoa[100];
    const char *p;

    snprintf(khoa, sizeof(khoa), "\"%s\"", tenTruong);

    p = strstr(json, khoa);
    if (p == NULL) return 0;

    p = strchr(p, ':');
    if (p == NULL) return 0;

    p++;

    while (*p && isspace((unsigned char)*p)) p++;

    return atoi(p);
}

/* Lay Top 10 nguoi choi */
int layBangXepHang(
    NguoiChoiXepHang danhSach[],
    int soLuongToiDa,
    int *soLuongThucTe,
    char *phanHoi,
    DWORD kichThuocPhanHoi
) {
    char duLieu[16000];
    DWORD maTrangThai = 0;
    const char *token = NULL;

    char *p;
    int dem = 0;

    if (soLuongThucTe != NULL) {
        *soLuongThucTe = 0;
    }

    if (danhSach == NULL || soLuongToiDa <= 0 ||
        phanHoi == NULL || kichThuocPhanHoi == 0) {
        return 0;
    }

    phanHoi[0] = '\0';

    /* Neu nguoi choi X da dang nhap thi dung token */
    if (nguoiChoiDaDangNhap(1)) {
        token = accessToken[0];
    }

    /* Lay danh sach profiles sap xep theo diem giam dan */
    if (!guiYeuCauSupabase(
        L"GET",
        L"/rest/v1/profiles?select=username,rank_points,total_matches,total_wins&order=rank_points.desc&limit=10",
        NULL,
        token,
        duLieu,
        sizeof(duLieu),
        &maTrangThai
    )) {
        snprintf(phanHoi, kichThuocPhanHoi,
            "Khong lay duoc bang xep hang. HTTP: %lu. Phan hoi: %.250s",
            (unsigned long)maTrangThai, duLieu);
        return 0;
    }

    /* Doc tung object trong mang JSON */
    p = duLieu;

    while (*p && dem < soLuongToiDa && dem < SO_HANG_XEP_HANG) {

        char *dauNgoac = strchr(p, '{');
        char *cuoiNgoac;

        if (dauNgoac == NULL) break;

        cuoiNgoac = strchr(dauNgoac, '}');
        if (cuoiNgoac == NULL) break;

        {
            char object[1500];
            size_t doDai = (size_t)(cuoiNgoac - dauNgoac + 1);

            if (doDai >= sizeof(object)) {
                p = cuoiNgoac + 1;
                continue;
            }

            memcpy(object, dauNgoac, doDai);
            object[doDai] = '\0';

            memset(&danhSach[dem], 0, sizeof(NguoiChoiXepHang));

            if (layChuoiJSON(
                object,
                "username",
                danhSach[dem].username,
                sizeof(danhSach[dem].username)
            )) {
                danhSach[dem].rank_points =
                    laySoJSON(object, "rank_points");

                danhSach[dem].total_matches =
                    laySoJSON(object, "total_matches");

                danhSach[dem].total_wins =
                    laySoJSON(object, "total_wins");

                dem++;
            }
        }

        p = cuoiNgoac + 1;
    }

    if (soLuongThucTe != NULL) {
        *soLuongThucTe = dem;
    }

    snprintf(phanHoi, kichThuocPhanHoi,
        "Da lay duoc %d nguoi choi.", dem);

    return 1;
}


