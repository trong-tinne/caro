#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <windows.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "supabase.h"

/* =====================================================
   1. CAU HINH GAME
   ===================================================== */

#define SO_O 15
#define KICH_THUOC_O 40
#define LE 30

#define RONG (SO_O * KICH_THUOC_O + LE * 2)
#define CAO  (SO_O * KICH_THUOC_O + LE * 2)

#define MENU_CUA_SO_RONG 1000
#define MENU_CUA_SO_CAO 700
#define MENU_X_MAX 0.48

/* =====================================================
   2. BIEN TOAN CUC
   ===================================================== */

char banco[SO_O][SO_O];

char nguoiChoi = 'X';
int ketThuc = 0;

/*
   1: AI Kha
   2: AI Kho
   3: Hai nguoi choi
*/

int cheDo = 1;

SDL_Texture *anhMenu = NULL;

int menuRong = 1000;
int menuCao = 700;

SDL_Rect viTriMenu;

/* =====================================================
   3. KHAI BAO HAM
   ===================================================== */

void lamMoiBanCo(void);
void veBanCo(SDL_Renderer *renderer);

int demLienTiep(int dong, int cot, int dx, int dy, char quan);
int kiemTraThang(int dong, int cot, char quan);
int banCoDay(void);

int timNuocThang(char quan, int *dong, int *cot);

int diemMotHuong(int dong, int cot, int dx, int dy, char quan);
int tinhDiem(int dong, int cot, char quan);

void timNuocAI(int *dongAI, int *cotAI, int mucDo);

void thongBao(SDL_Window *window, const char *noiDung);

void taiAnhMenu(SDL_Renderer *renderer);
void veMenu(SDL_Renderer *renderer);

int xuLyClickMenu(int x, int y);
int chonCheDo(SDL_Window *window);

void nhapChuoi(const char *thongBaoNhap, char *chuoi, int kichThuoc);

/* Da sua: ham co them tham so cheDoDangChon */
void dangNhapGiaoDien(SDL_Window *window, int cheDoDangChon);

void dangKyGiaoDien(SDL_Window *window);

void luuKetQuaTranDau(SDL_Window *window, const char *ketQua);
void hienThiBangXepHang(SDL_Window *window);


/* =====================================================
   4. LAM MOI BAN CO
   ===================================================== */

void lamMoiBanCo(void) {
    int i, j;

    for (i = 0; i < SO_O; i++) {
        for (j = 0; j < SO_O; j++) {
            banco[i][j] = '.';
        }
    }

    nguoiChoi = 'X';
    ketThuc = 0;
}


/* =====================================================
   5. TAI ANH MENU
   ===================================================== */

void taiAnhMenu(SDL_Renderer *renderer) {

    SDL_Surface *anh = IMG_Load("menu.png");

    if (anh == NULL) {
        printf("Khong the tai menu.png: %s\n", IMG_GetError());
        return;
    }

    anhMenu = SDL_CreateTextureFromSurface(renderer, anh);

    if (anhMenu == NULL) {
        printf("Loi tao texture menu: %s\n", SDL_GetError());
    } else {
        menuRong = anh->w;
        menuCao = anh->h;

        printf("Da tai menu.png thanh cong! Kich thuoc: %d x %d\n",
               menuRong, menuCao);
    }

    SDL_FreeSurface(anh);
}


/* =====================================================
   6. VE MENU
   ===================================================== */

void veMenu(SDL_Renderer *renderer) {

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (anhMenu != NULL && menuRong > 0 && menuCao > 0) {

        float tiLeRong = (float)MENU_CUA_SO_RONG / menuRong;
        float tiLeCao = (float)MENU_CUA_SO_CAO / menuCao;

        float tiLe = (tiLeRong < tiLeCao) ? tiLeRong : tiLeCao;

        viTriMenu.w = (int)(menuRong * tiLe);
        viTriMenu.h = (int)(menuCao * tiLe);

        viTriMenu.x = (MENU_CUA_SO_RONG - viTriMenu.w) / 2;
        viTriMenu.y = (MENU_CUA_SO_CAO - viTriMenu.h) / 2;

        SDL_RenderCopy(renderer, anhMenu, NULL, &viTriMenu);

    } else {

        SDL_SetRenderDrawColor(renderer, 245, 222, 179, 255);
        SDL_RenderClear(renderer);
    }

    SDL_RenderPresent(renderer);
}


/* =====================================================
   7. XU LY CLICK MENU
   ===================================================== */

int xuLyClickMenu(int x, int y) {

    int toaDoAnhX;
    int toaDoAnhY;
    int luaChon = 0;

    if (anhMenu == NULL || viTriMenu.w <= 0 || viTriMenu.h <= 0) {
        return 0;
    }

    if (x < viTriMenu.x || x >= viTriMenu.x + viTriMenu.w ||
        y < viTriMenu.y || y >= viTriMenu.y + viTriMenu.h) {
        return 0;
    }

    toaDoAnhX = (int)((x - viTriMenu.x) *
                (float)menuRong / viTriMenu.w);

    toaDoAnhY = (int)((y - viTriMenu.y) *
                (float)menuCao / viTriMenu.h);

    if (toaDoAnhX < 0 || toaDoAnhX > menuRong * MENU_X_MAX) {
        return 0;
    }

    if (toaDoAnhY >= 300 && toaDoAnhY < 436)
        luaChon = 1;

    else if (toaDoAnhY >= 436 && toaDoAnhY < 547)
        luaChon = 2;

    else if (toaDoAnhY >= 547 && toaDoAnhY < 644)
        luaChon = 3;

    else if (toaDoAnhY >= 644 && toaDoAnhY < 742)
        luaChon = 4;

    else if (toaDoAnhY >= 742 && toaDoAnhY < 845)
        luaChon = 5;

    else if (toaDoAnhY >= 845 && toaDoAnhY <= 1000)
        luaChon = 6;

    printf("Toa do anh: X=%d, Y=%d; nut=%d\n",
           toaDoAnhX, toaDoAnhY, luaChon);

    fflush(stdout);

    return luaChon;
}


/* =====================================================
   8. VE BAN CO
   ===================================================== */

void veBanCo(SDL_Renderer *renderer) {

    int i, j, g;

    SDL_SetRenderDrawColor(renderer, 245, 222, 179, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);

    /* Ve cac duong ke */
    for (i = 0; i <= SO_O; i++) {

        int toaDo = LE + i * KICH_THUOC_O;

        SDL_RenderDrawLine(renderer,
            LE, toaDo,
            LE + SO_O * KICH_THUOC_O, toaDo);

        SDL_RenderDrawLine(renderer,
            toaDo, LE,
            toaDo, LE + SO_O * KICH_THUOC_O);
    }

    /* Ve quan co */
    for (i = 0; i < SO_O; i++) {

        for (j = 0; j < SO_O; j++) {

            int x = LE + j * KICH_THUOC_O + KICH_THUOC_O / 2;
            int y = LE + i * KICH_THUOC_O + KICH_THUOC_O / 2;

            /* Quan X */
            if (banco[i][j] == 'X') {

                SDL_SetRenderDrawColor(renderer, 220, 30, 30, 255);

                SDL_RenderDrawLine(renderer,
                    x - 12, y - 12,
                    x + 12, y + 12);

                SDL_RenderDrawLine(renderer,
                    x + 12, y - 12,
                    x - 12, y + 12);
            }

            /* Quan O */
            else if (banco[i][j] == 'O') {

                SDL_SetRenderDrawColor(renderer, 30, 70, 220, 255);

                for (g = 0; g < 360; g++) {

                    double a1 = g * 3.14159265 / 180.0;
                    double a2 = (g + 1) * 3.14159265 / 180.0;

                    SDL_RenderDrawLine(renderer,
                        x + (int)(12 * cos(a1)),
                        y + (int)(12 * sin(a1)),

                        x + (int)(12 * cos(a2)),
                        y + (int)(12 * sin(a2)));
                }
            }
        }
    }

    SDL_RenderPresent(renderer);
}


/* =====================================================
   9. DEM QUAN LIEN TIEP
   ===================================================== */

int demLienTiep(int dong, int cot, int dx, int dy, char quan) {

    int dem = 1;
    int i, j;

    /* Dem theo chieu duong */
    i = dong + dx;
    j = cot + dy;

    while (i >= 0 && i < SO_O &&
           j >= 0 && j < SO_O &&
           banco[i][j] == quan) {

        dem++;
        i += dx;
        j += dy;
    }

    /* Dem theo chieu am */
    i = dong - dx;
    j = cot - dy;

    while (i >= 0 && i < SO_O &&
           j >= 0 && j < SO_O &&
           banco[i][j] == quan) {

        dem++;
        i -= dx;
        j -= dy;
    }

    return dem;
}


/* =====================================================
   10. KIEM TRA THANG / HOA
   ===================================================== */

int kiemTraThang(int dong, int cot, char quan) {

    if (demLienTiep(dong, cot, 0, 1, quan) >= 5)
        return 1;

    if (demLienTiep(dong, cot, 1, 0, quan) >= 5)
        return 1;

    if (demLienTiep(dong, cot, 1, 1, quan) >= 5)
        return 1;

    if (demLienTiep(dong, cot, 1, -1, quan) >= 5)
        return 1;

    return 0;
}


int banCoDay(void) {

    int i, j;

    for (i = 0; i < SO_O; i++) {
        for (j = 0; j < SO_O; j++) {

            if (banco[i][j] == '.') {
                return 0;
            }
        }
    }

    return 1;
}


/* =====================================================
   11. TIM NUOC THANG NGAY
   ===================================================== */

int timNuocThang(char quan, int *dong, int *cot) {

    int i, j;

    for (i = 0; i < SO_O; i++) {

        for (j = 0; j < SO_O; j++) {

            if (banco[i][j] == '.') {

                banco[i][j] = quan;

                if (kiemTraThang(i, j, quan)) {

                    banco[i][j] = '.';

                    *dong = i;
                    *cot = j;

                    return 1;
                }

                banco[i][j] = '.';
            }
        }
    }

    return 0;
}


/* =====================================================
   12. TINH DIEM AI
   ===================================================== */

int diemMotHuong(int dong, int cot,
                 int dx, int dy, char quan) {

    int dem = 1;
    int dauMo = 0;
    int i, j;

    i = dong + dx;
    j = cot + dy;

    while (i >= 0 && i < SO_O &&
           j >= 0 && j < SO_O &&
           banco[i][j] == quan) {

        dem++;
        i += dx;
        j += dy;
    }

    if (i >= 0 && i < SO_O &&
        j >= 0 && j < SO_O &&
        banco[i][j] == '.') {

        dauMo++;
    }

    i = dong - dx;
    j = cot - dy;

    while (i >= 0 && i < SO_O &&
           j >= 0 && j < SO_O &&
           banco[i][j] == quan) {

        dem++;
        i -= dx;
        j -= dy;
    }

    if (i >= 0 && i < SO_O &&
        j >= 0 && j < SO_O &&
        banco[i][j] == '.') {

        dauMo++;
    }

    if (dem >= 5) return 100000;

    if (dem == 4 && dauMo == 2) return 10000;
    if (dem == 4 && dauMo == 1) return 5000;

    if (dem == 3 && dauMo == 2) return 1000;
    if (dem == 3 && dauMo == 1) return 200;

    if (dem == 2 && dauMo == 2) return 100;
    if (dem == 2 && dauMo == 1) return 20;

    return dem;
}


int tinhDiem(int dong, int cot, char quan) {

    int diem = 0;

    diem += diemMotHuong(dong, cot, 0, 1, quan);
    diem += diemMotHuong(dong, cot, 1, 0, quan);
    diem += diemMotHuong(dong, cot, 1, 1, quan);
    diem += diemMotHuong(dong, cot, 1, -1, quan);

    return diem;
}


/* =====================================================
   13. AI CHON NUOC DI
   ===================================================== */

void timNuocAI(int *dongAI, int *cotAI, int mucDo) {

    int i, j;

    *dongAI = -1;
    *cotAI = -1;

    /* Neu AI co nuoc thang thi danh ngay */
    if (timNuocThang('O', dongAI, cotAI))
        return;

    /* Neu X sap thang thi chan */
    if (timNuocThang('X', dongAI, cotAI))
        return;

    {
        int diem1 = -1;
        int diem2 = -1;
        int diem3 = -1;

        int hang1 = SO_O / 2;
        int cot1 = SO_O / 2;

        int hang2 = hang1;
        int cot2 = cot1;

        int hang3 = hang1;
        int cot3 = cot1;

        for (i = 0; i < SO_O; i++) {

            for (j = 0; j < SO_O; j++) {

                if (banco[i][j] == '.') {

                    int diem;

                    banco[i][j] = 'O';
                    diem = tinhDiem(i, j, 'O');
                    banco[i][j] = '.';

                    banco[i][j] = 'X';
                    diem += tinhDiem(i, j, 'X') * 2;
                    banco[i][j] = '.';

                    diem += SO_O -
                        (abs(i - SO_O / 2) +
                         abs(j - SO_O / 2)) / 2;

                    if (diem > diem1) {

                        diem3 = diem2;
                        hang3 = hang2;
                        cot3 = cot2;

                        diem2 = diem1;
                        hang2 = hang1;
                        cot2 = cot1;

                        diem1 = diem;
                        hang1 = i;
                        cot1 = j;
                    }

                    else if (diem > diem2) {

                        diem3 = diem2;
                        hang3 = hang2;
                        cot3 = cot2;

                        diem2 = diem;
                        hang2 = i;
                        cot2 = j;
                    }

                    else if (diem > diem3) {

                        diem3 = diem;
                        hang3 = i;
                        cot3 = j;
                    }
                }
            }
        }

        /* AI Kha: chon ngau nhien trong 3 nuoc tot */
        if (mucDo == 1) {

            int luaChon = rand() % 3;

            if (luaChon == 0 || diem2 == -1) {

                *dongAI = hang1;
                *cotAI = cot1;
            }

            else if (luaChon == 1 || diem3 == -1) {

                *dongAI = hang2;
                *cotAI = cot2;
            }

            else {

                *dongAI = hang3;
                *cotAI = cot3;
            }
        }

        /* AI Kho: chon nuoc co diem cao nhat */
        else {

            *dongAI = hang1;
            *cotAI = cot1;
        }
    }
}


/* =====================================================
   14. THONG BAO VA CHON CHE DO
   ===================================================== */

void thongBao(SDL_Window *window, const char *noiDung) {

    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_INFORMATION,
        "GAME CARO",
        noiDung,
        window
    );
}


int chonCheDo(SDL_Window *window) {

    const SDL_MessageBoxButtonData nut[] = {

        {0, 1, "AI Kha"},
        {0, 2, "AI Kho"},
        {0, 3, "Hai nguoi choi"},

        {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 4, "Quay lai"}
    };

    SDL_MessageBoxData menu;

    int luaChon = 4;

    menu.flags = SDL_MESSAGEBOX_INFORMATION;
    menu.window = window;
    menu.title = "GAME CARO";
    menu.message = "Chon che do choi:";
    menu.numbuttons = 4;
    menu.buttons = nut;
    menu.colorScheme = NULL;

    if (SDL_ShowMessageBox(&menu, &luaChon) < 0)
        return 0;

    if (luaChon >= 1 && luaChon <= 3) {

        cheDo = luaChon;

        return 1;
    }

    return 0;
}


/* =====================================================
   15. NHAP CHUOI CONSOLE
   ===================================================== */

void nhapChuoi(const char *thongBaoNhap,
               char *chuoi,
               int kichThuoc) {

    printf("%s", thongBaoNhap);

    fflush(stdout);

    if (fgets(chuoi, kichThuoc, stdin) != NULL) {

        chuoi[strcspn(chuoi, "\r\n")] = '\0';
    }

    else {

        chuoi[0] = '\0';
    }
}


/* =====================================================
   16. DANG NHAP TAI KHOAN
   ===================================================== */

/*
   cheDoDangChon:

   0: Dang nhap tu menu
   1: AI Kha
   2: AI Kho
   3: Hai nguoi choi

*/

void dangNhapGiaoDien(SDL_Window *window, int cheDoDangChon) {

    char email[256];
    char matKhau[256];

    char phanHoi[4000];
    char luaChon[16];

    printf("\n========== DANG NHAP GAME CARO ==========\n");

    /* Dang nhap tai khoan X neu chua dang nhap */
    if (!nguoiChoiDaDangNhap(1)) {

        printf("\nDang nhap tai khoan nguoi choi X:\n");

        nhapChuoi("Nhap email X: ", email, sizeof(email));

        nhapChuoi("Nhap mat khau X: ", matKhau, sizeof(matKhau));

        if (!dangNhapNguoiChoi(
                1, email, matKhau,
                phanHoi, sizeof(phanHoi))) {

            printf("Dang nhap X that bai: %s\n", phanHoi);

            matKhau[0] = '\0';

            thongBao(window,
                "Dang nhap tai khoan X that bai. Hay xem cua so den!");

            return;
        }

        matKhau[0] = '\0';

        printf("X dang nhap thanh cong: %s (%s)\n",
               layTenNguoiChoi(1),
               layIDNguoiChoi(1));
    }

    else {

        printf("Tai khoan X da dang nhap: %s\n",
               layTenNguoiChoi(1));
    }

    /* Neu choi voi AI thi khong can tai khoan O */
    if (cheDoDangChon == 1 || cheDoDangChon == 2) {

        dangXuatNguoiChoi(2);

        thongBao(window,
            "Dang nhap X thanh cong! Ban co the choi voi AI.");

        return;
    }

    /* Neu choi hai nguoi thi bat buoc dang nhap O */
    if (cheDoDangChon == 3) {

        if (!nguoiChoiDaDangNhap(2)) {

            printf("\nDang nhap tai khoan nguoi choi O:\n");

            nhapChuoi("Nhap email O: ", email, sizeof(email));

            nhapChuoi("Nhap mat khau O: ", matKhau, sizeof(matKhau));

            if (!dangNhapNguoiChoi(
                    2, email, matKhau,
                    phanHoi, sizeof(phanHoi))) {

                printf("Dang nhap O that bai: %s\n", phanHoi);

                dangXuatNguoiChoi(2);

                matKhau[0] = '\0';

                thongBao(window,
                    "Dang nhap O that bai. "
                    "Che do 2 nguoi can ca hai tai khoan!");

                return;
            }

            matKhau[0] = '\0';

            printf("O dang nhap thanh cong: %s (%s)\n",
                   layTenNguoiChoi(2),
                   layIDNguoiChoi(2));
        }

        thongBao(window,
            "Ca hai tai khoan X va O da dang nhap thanh cong!");

        return;
    }

    /* Dang nhap tu menu */
    nhapChuoi(
        "Ban co muon dang nhap them tai khoan O? (Y/N): ",
        luaChon,
        sizeof(luaChon)
    );

    if (luaChon[0] == 'Y' || luaChon[0] == 'y') {

        dangNhapGiaoDien(window, 3);
    }

    else {

        dangXuatNguoiChoi(2);

        thongBao(window,
            "Tai khoan X da dang nhap thanh cong!");
    }
}


/* =====================================================
   17. DANG KY SUPABASE
   ===================================================== */

void dangKyGiaoDien(SDL_Window *window) {

    char email[256];
    char matKhau[256];
    char tenDangNhap[256];

    char phanHoi[4000];

    printf("\n========== DANG KY TAI KHOAN ==========\n");

    nhapChuoi("Nhap ten dang nhap: ",
              tenDangNhap, sizeof(tenDangNhap));

    nhapChuoi("Nhap email: ",
              email, sizeof(email));

    nhapChuoi("Nhap mat khau: ",
              matKhau, sizeof(matKhau));

    if (dangKyTaiKhoan(
            email,
            matKhau,
            tenDangNhap,
            phanHoi,
            sizeof(phanHoi))) {

        thongBao(window,
            "Gui yeu cau dang ky thanh cong! "
            "Hay kiem tra email neu can xac minh.");
    }

    else {

        printf("Phan hoi Supabase: %s\n", phanHoi);

        thongBao(window,
            "Dang ky that bai. Hay kiem tra cua so den!");
    }

    matKhau[0] = '\0';
}


/* =====================================================
   18. LUU TRAN DAU
   ===================================================== */

void luuKetQuaTranDau(SDL_Window *window, const char *ketQua) {

    char phanHoi[4000];

    const char *tenCheDo;

    if (cheDo == 3) {

        /* Tran hai nguoi can ca X va O dang nhap */

        if (!nguoiChoiDaDangNhap(1) ||
            !nguoiChoiDaDangNhap(2)) {

            printf("Khong luu: chua dang nhap du hai tai khoan.\n");

            thongBao(window,
                "Tran dau khong duoc luu vi chua dang nhap du hai tai khoan!");

            return;
        }

        tenCheDo = "2_PLAYER";
    }

    else {

        /* Tran voi AI chi can tai khoan X */

        if (!nguoiChoiDaDangNhap(1)) {

            printf("Khong luu: nguoi choi X chua dang nhap.\n");

            thongBao(window,
                "Tran dau khong duoc luu vi ban chua dang nhap!");

            return;
        }

        tenCheDo = (cheDo == 1) ? "AI_KHA" : "AI_KHO";
    }

    printf("\n========== LUU TRAN DAU ==========\n");

    printf("Che do: %s | Ket qua: %s\n",
           tenCheDo, ketQua);

    if (ghiNhanTranDau(
            ketQua,
            tenCheDo,
            phanHoi,
            sizeof(phanHoi))) {

        printf("Luu tran dau thanh cong. Phan hoi: %s\n", phanHoi);

        thongBao(window,
            "Da luu tran dau va cap nhat diem rank thanh cong!");
    }

    else {

        printf("Luu tran dau that bai: %s\n", phanHoi);

        thongBao(window,
            "Luu tran dau that bai. "
            "Hay kiem tra cua so den va cau hinh Supabase!");
    }

    fflush(stdout);
}

/* =====================================================
   HIEN THI BANG XEP HANG TOP 10
   ===================================================== */

void hienThiBangXepHang(SDL_Window *window) {

    NguoiChoiXepHang danhSach[SO_HANG_XEP_HANG];

    int soLuong = 0;
    int i;

    char phanHoi[4000];
    char noiDung[4000];

    memset(danhSach, 0, sizeof(danhSach));

    printf("\n========== TAI BANG XEP HANG ==========\n");

    if (!layBangXepHang(
        danhSach,
        SO_HANG_XEP_HANG,
        &soLuong,
        phanHoi,
        sizeof(phanHoi)
    )) {

        printf("Loi: %s\n", phanHoi);

        thongBao(window,
            "Khong the tai bang xep hang!\n"
            "Hay kiem tra ket noi va quyen doc profiles.");

        return;
    }

    if (soLuong == 0) {

        thongBao(window,
            "Chua co nguoi choi nao trong bang xep hang.");

        return;
    }

    snprintf(noiDung, sizeof(noiDung),
        "TOP 10 NGUOI CHOI CARO\n"
        "--------------------------------\n"
        "Hang | Ten | Rank | Tran | Thang\n"
        "--------------------------------\n");

    for (i = 0; i < soLuong; i++) {

        char dong[350];

        snprintf(dong, sizeof(dong),
            "%2d. %-18.18s | %5d | %4d | %4d\n",
            i + 1,
            danhSach[i].username,
            danhSach[i].rank_points,
            danhSach[i].total_matches,
            danhSach[i].total_wins
        );

        if (strlen(noiDung) + strlen(dong) + 1 <
            sizeof(noiDung)) {

            strcat(noiDung, dong);
        }
    }

    strcat(noiDung,
        "--------------------------------\n"
        "Nhan OK de quay lai menu.");

    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_INFORMATION,
        "BANG XEP HANG GAME CARO",
        noiDung,
        window
    );
}



/* =====================================================
   19. CHUONG TRINH CHINH
   ===================================================== */

int main(int argc, char *argv[]) {

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    SDL_Event event;

    int chay = 1;
    int dangChoi = 0;

    int dongAI = -1;
    int cotAI = -1;

    srand((unsigned int)time(NULL));

    /* Khoi tao SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {

        printf("Loi SDL: %s\n", SDL_GetError());

        return 1;
    }

    /* Khoi tao SDL_image */
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {

        printf("Loi SDL_image: %s\n", IMG_GetError());

        SDL_Quit();

        return 1;
    }

    /* Kiem tra ket noi Supabase */
    if (kiemTraSupabase()) {

        printf("Ket noi Supabase thanh cong!\n");
    }

    else {

        printf("Khong the ket noi Supabase!\n");
    }

    /* Tao cua so game */
    window = SDL_CreateWindow(
        "GAME CARO",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        MENU_CUA_SO_RONG,
        MENU_CUA_SO_CAO,
        SDL_WINDOW_SHOWN
    );

    if (window == NULL) {

        printf("Loi tao cua so: %s\n", SDL_GetError());

        IMG_Quit();
        SDL_Quit();

        return 1;
    }

    /* Tao renderer */
    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (renderer == NULL) {

        renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_SOFTWARE
        );
    }

    if (renderer == NULL) {

        printf("Loi renderer: %s\n", SDL_GetError());

        SDL_DestroyWindow(window);

        IMG_Quit();
        SDL_Quit();

        return 1;
    }

    /* Tai anh menu */
    taiAnhMenu(renderer);

    if (anhMenu == NULL) {

        thongBao(window,
            "Khong tim thay menu.png. "
            "Hay dat anh cung thu muc voi file EXE.");
    }

    lamMoiBanCo();


    /* =================================================
       VONG LAP MENU CHINH
       ================================================= */

    while (chay) {

        if (!dangChoi) {

            int daChon = 0;

            veMenu(renderer);

            while (!daChon && chay) {

                while (SDL_PollEvent(&event)) {

                    if (event.type == SDL_QUIT) {

                        chay = 0;

                        break;
                    }

                    if (event.type == SDL_MOUSEBUTTONDOWN &&
                        event.button.button == SDL_BUTTON_LEFT) {

                        /*
                           Lua chon menu phai duoc khai bao
                           ben trong vung xu ly chuot.
                        */

                        int luaChon = xuLyClickMenu(
                            event.button.x,
                            event.button.y
                        );


                        /* =================================
                           NUT 1: CHOI NGAY
                           ================================= */

                        if (luaChon == 1) {

                            if (chonCheDo(window)) {

                                /* Neu chon hai nguoi */
                                if (cheDo == 3) {

                                    if (!nguoiChoiDaDangNhap(1) ||
                                        !nguoiChoiDaDangNhap(2)) {

                                        dangNhapGiaoDien(window, 3);
                                    }

                                    /* Kiem tra lai sau dang nhap */
                                    if (nguoiChoiDaDangNhap(1) &&
                                        nguoiChoiDaDangNhap(2)) {

                                        lamMoiBanCo();

                                        SDL_SetWindowSize(
                                            window,
                                            RONG,
                                            CAO
                                        );

                                        dangChoi = 1;
                                    }
                                }

                                /* Neu chon AI */
                                else {

                                    if (!nguoiChoiDaDangNhap(1)) {

                                        dangNhapGiaoDien(window, cheDo);
                                    }

                                    /* AI chi can tai khoan X */
                                    if (nguoiChoiDaDangNhap(1)) {

                                        dangXuatNguoiChoi(2);

                                        lamMoiBanCo();

                                        SDL_SetWindowSize(
                                            window,
                                            RONG,
                                            CAO
                                        );

                                        dangChoi = 1;
                                    }
                                }
                            }

                            daChon = 1;
                        }


                        /* =================================
                           NUT 2: DANG NHAP
                           ================================= */

                        else if (luaChon == 2) {

                            dangNhapGiaoDien(window, 0);

                            veMenu(renderer);

                            daChon = 1;
                        }


                        /* =================================
                           NUT 3: DANG KY
                           ================================= */

                        else if (luaChon == 3) {

                            dangKyGiaoDien(window);

                            veMenu(renderer);

                            daChon = 1;
                        }


                        /* =================================
                           NUT 4: BANG XEP HANG
                           ================================= */

                        else if (luaChon == 4) {

    hienThiBangXepHang(window);

    veMenu(renderer);

    daChon = 1;
}


                        /* =================================
                           NUT 5: HUONG DAN
                           ================================= */

                        else if (luaChon == 5) {

                            thongBao(
                                window,

                                "LUAT CHOI CARO:\n\n"

                                "1. Nguoi choi X di truoc.\n"

                                "2. Hai nguoi lan luot danh co.\n"

                                "3. Nguoi tao duoc 5 quan lien tiep se thang.\n"

                                "4. Bam ESC de quay lai menu.\n"

                                "5. Dang nhap X de luu tran dau voi AI.\n"

                                "6. Dang nhap ca X va O de choi 2 nguoi."
                            );

                            veMenu(renderer);

                            daChon = 1;
                        }


                        /* =================================
                           NUT 6: THOAT
                           ================================= */

                        else if (luaChon == 6) {

                            chay = 0;

                            daChon = 1;
                        }
                    }
                }

                SDL_Delay(10);
            }
        }


        /* =================================================
           VONG LAP TRAN DAU
           ================================================= */

        while (chay && dangChoi) {

            while (SDL_PollEvent(&event)) {

                /* Thoat game */
                if (event.type == SDL_QUIT) {

                    chay = 0;
                    dangChoi = 0;

                    break;
                }

                /* ESC quay lai menu */
                if (event.type == SDL_KEYDOWN &&
                    event.key.keysym.sym == SDLK_ESCAPE) {

                    dangChoi = 0;

                    SDL_SetWindowSize(
                        window,
                        MENU_CUA_SO_RONG,
                        MENU_CUA_SO_CAO
                    );

                    break;
                }


                /* Nhan chuot de danh co */
                if (event.type == SDL_MOUSEBUTTONDOWN &&
                    event.button.button == SDL_BUTTON_LEFT &&
                    !ketThuc) {

                    int x = event.button.x;
                    int y = event.button.y;

                    if (x >= LE &&
                        x < LE + SO_O * KICH_THUOC_O &&
                        y >= LE &&
                        y < LE + SO_O * KICH_THUOC_O) {

                        int cot = (x - LE) / KICH_THUOC_O;
                        int dong = (y - LE) / KICH_THUOC_O;

                        if (banco[dong][cot] == '.') {

                            char quanVuaDanh = nguoiChoi;

                            banco[dong][cot] = quanVuaDanh;


                            /* =================================
                               KIEM TRA NGUOI VUA DANH THANG
                               ================================= */

                            if (kiemTraThang(
                                    dong,
                                    cot,
                                    quanVuaDanh)) {

                                ketThuc = 1;

                                if (cheDo == 3) {

                                    if (quanVuaDanh == 'X') {

                                        luuKetQuaTranDau(
                                            window,
                                            "X_WIN"
                                        );

                                        thongBao(
                                            window,
                                            "Nguoi choi X thang!"
                                        );
                                    }

                                    else {

                                        luuKetQuaTranDau(
                                            window,
                                            "O_WIN"
                                        );

                                        thongBao(
                                            window,
                                            "Nguoi choi O thang!"
                                        );
                                    }
                                }

                                else {

                                    luuKetQuaTranDau(
                                        window,
                                        "X_WIN"
                                    );

                                    thongBao(
                                        window,
                                        "Ban da thang AI!"
                                    );
                                }
                            }


                            /* =================================
                               KIEM TRA HOA SAU NUOC NGUOI
                               ================================= */

                            else if (banCoDay()) {

                                ketThuc = 1;

                                luuKetQuaTranDau(
                                    window,
                                    "DRAW"
                                );

                                thongBao(
                                    window,
                                    "Hoa! Ban co da day."
                                );
                            }


                            /* =================================
                               AI DANH QUAN O
                               ================================= */

                            else if (cheDo == 1 || cheDo == 2) {

                                timNuocAI(
                                    &dongAI,
                                    &cotAI,
                                    cheDo
                                );

                                if (dongAI >= 0 &&
                                    dongAI < SO_O &&
                                    cotAI >= 0 &&
                                    cotAI < SO_O &&
                                    banco[dongAI][cotAI] == '.') {

                                    banco[dongAI][cotAI] = 'O';

                                    if (kiemTraThang(
                                            dongAI,
                                            cotAI,
                                            'O')) {

                                        ketThuc = 1;

                                        /* O la AI, nguoi X thua */
                                        luuKetQuaTranDau(
                                            window,
                                            "O_WIN"
                                        );

                                        thongBao(
                                            window,
                                            "AI thang! Ban bi tru 10 diem rank."
                                        );
                                    }

                                    else if (banCoDay()) {

                                        ketThuc = 1;

                                        luuKetQuaTranDau(
                                            window,
                                            "DRAW"
                                        );

                                        thongBao(
                                            window,
                                            "Hoa voi AI! Ban duoc cong 5 diem rank."
                                        );
                                    }
                                }
                            }


                            /* =================================
                               HAI NGUOI CHOI THAY PHIEN
                               ================================= */

                            else {

                                nguoiChoi =
                                    (nguoiChoi == 'X') ? 'O' : 'X';
                            }
                        }
                    }
                }
            }


            /* Ve lai ban co */
            if (chay && dangChoi) {

                veBanCo(renderer);
            }


            /* Neu tran dau ket thuc */
            if (ketThuc) {

                SDL_Delay(300);

                dangChoi = 0;

                SDL_SetWindowSize(
                    window,
                    MENU_CUA_SO_RONG,
                    MENU_CUA_SO_CAO
                );

                lamMoiBanCo();

                break;
            }

            SDL_Delay(10);
        }
    }


    /* =================================================
       GIAI PHONG BO NHO
       ================================================= */

    if (anhMenu != NULL) {

        SDL_DestroyTexture(anhMenu);

        anhMenu = NULL;
    }

    SDL_DestroyRenderer(renderer);

    SDL_DestroyWindow(window);

    IMG_Quit();

    SDL_Quit();

    return 0;
}
