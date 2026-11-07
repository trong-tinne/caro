
-- =====================================================
-- GAME CARO - DATABASE SQL
-- SUPABASE / POSTGRESQL
-- =====================================================
-- Bao gom:
-- 1. Tables: profiles, matches, caro_rooms
-- 2. Primary keys, foreign keys, constraints
-- 3. Sequence matches_id_seq
-- 4. Functions / RPC
-- 5. Triggers
-- 6. Row Level Security (RLS)
-- 7. Realtime cho caro_rooms
--
-- Luu y:
-- - Can chay trong Supabase SQL Editor.
-- - Can Supabase Auth (auth.users, auth.uid()).
-- - Khong bao gom du lieu tai khoan nguoi dung.
-- =====================================================


-- =====================================================
-- 1. SEQUENCE
-- =====================================================

CREATE SEQUENCE IF NOT EXISTS public.matches_id_seq;


-- =====================================================
-- 2. TABLE PROFILES
-- =====================================================

CREATE TABLE IF NOT EXISTS public.profiles (
    id uuid NOT NULL,
    username text NOT NULL,
    rank_points integer NOT NULL DEFAULT 1000,
    total_matches integer NOT NULL DEFAULT 0,
    total_wins integer NOT NULL DEFAULT 0,
    total_losses integer NOT NULL DEFAULT 0,
    total_draws integer NOT NULL DEFAULT 0,
    created_at timestamp with time zone NOT NULL DEFAULT now(),

    CONSTRAINT profiles_pkey PRIMARY KEY (id),
    CONSTRAINT profiles_username_key UNIQUE (username),

    CONSTRAINT profiles_id_fkey
        FOREIGN KEY (id)
        REFERENCES auth.users(id)
        ON DELETE CASCADE,

    CONSTRAINT profiles_rank_points_check
        CHECK (rank_points >= 0),

    CONSTRAINT profiles_username_check
        CHECK (
            char_length(username) >= 3
            AND char_length(username) <= 20
        )
);


-- =====================================================
-- 3. TABLE MATCHES
-- =====================================================

CREATE TABLE IF NOT EXISTS public.matches (
    id bigint NOT NULL DEFAULT nextval('public.matches_id_seq'::regclass),
    player_x uuid NOT NULL,
    player_o uuid,
    winner_id uuid,
    result text NOT NULL,
    mode text NOT NULL,
    played_at timestamp with time zone NOT NULL DEFAULT now(),

    CONSTRAINT matches_pkey PRIMARY KEY (id),

    CONSTRAINT matches_player_x_fkey
        FOREIGN KEY (player_x)
        REFERENCES public.profiles(id),

    CONSTRAINT matches_player_o_fkey
        FOREIGN KEY (player_o)
        REFERENCES public.profiles(id),

    CONSTRAINT matches_winner_id_fkey
        FOREIGN KEY (winner_id)
        REFERENCES public.profiles(id),

    CONSTRAINT matches_mode_check
        CHECK (
            mode = ANY (
                ARRAY['PVP', 'AI', 'AI_KHA', 'AI_KHO']::text[]
            )
        ),

    CONSTRAINT matches_result_check
        CHECK (
            result = ANY (
                ARRAY['X_WIN', 'O_WIN', 'DRAW']::text[]
            )
        ),

    CONSTRAINT matches_check
        CHECK (
            (
                mode = 'PVP'
                AND player_o IS NOT NULL
                AND player_x <> player_o
            )
            OR
            (
                mode = ANY (
                    ARRAY['AI', 'AI_KHA', 'AI_KHO']::text[]
                )
                AND player_o IS NULL
            )
        ),

    CONSTRAINT matches_check1
        CHECK (
            (
                result = 'X_WIN'
                AND winner_id = player_x
            )
            OR
            (
                result = 'O_WIN'
                AND mode = 'PVP'
                AND winner_id = player_o
            )
            OR
            (
                result = 'O_WIN'
                AND mode = ANY (
                    ARRAY['AI', 'AI_KHA', 'AI_KHO']::text[]
                )
                AND winner_id IS NULL
            )
            OR
            (
                result = 'DRAW'
                AND winner_id IS NULL
            )
        )
);

ALTER SEQUENCE public.matches_id_seq
OWNED BY public.matches.id;


-- =====================================================
-- 4. TABLE CARO_ROOMS
-- =====================================================

CREATE TABLE IF NOT EXISTS public.caro_rooms (
    id uuid NOT NULL DEFAULT gen_random_uuid(),
    room_code text NOT NULL,
    host_id uuid NOT NULL,
    guest_id uuid,
    board jsonb NOT NULL DEFAULT '{}'::jsonb,
    current_turn text NOT NULL DEFAULT 'X',
    status text NOT NULL DEFAULT 'waiting',
    result text,
    winner_id uuid,
    created_at timestamp with time zone NOT NULL DEFAULT now(),
    updated_at timestamp with time zone NOT NULL DEFAULT now(),

    CONSTRAINT caro_rooms_pkey PRIMARY KEY (id),

    CONSTRAINT caro_rooms_room_code_key
        UNIQUE (room_code),

    CONSTRAINT caro_rooms_host_id_fkey
        FOREIGN KEY (host_id)
        REFERENCES auth.users(id),

    CONSTRAINT caro_rooms_guest_id_fkey
        FOREIGN KEY (guest_id)
        REFERENCES auth.users(id),

    CONSTRAINT caro_rooms_winner_id_fkey
        FOREIGN KEY (winner_id)
        REFERENCES auth.users(id),

    CONSTRAINT valid_status
        CHECK (
            status = ANY (
                ARRAY['waiting', 'playing', 'finished']::text[]
            )
        ),

    CONSTRAINT valid_turn
        CHECK (
            current_turn = ANY (ARRAY['X', 'O']::text[])
        )
);


-- =====================================================
-- 5. FUNCTION HANDLE NEW USER
-- Tu dong tao profile khi dang ky tai khoan
-- =====================================================

CREATE OR REPLACE FUNCTION public.handle_new_user()
RETURNS trigger
LANGUAGE plpgsql
SECURITY DEFINER
SET search_path TO ''
AS $function$
BEGIN
    INSERT INTO public.profiles (id, username)
    VALUES (
        NEW.id,
        lower(
            coalesce(
                nullif(trim(NEW.raw_user_meta_data ->> 'username'), ''),
                'player_' || substr(NEW.id::text, 1, 8)
            )
        )
    );

    RETURN NEW;
END;
$function$;


-- =====================================================
-- 6. FUNCTION CAP NHAT RANK
-- =====================================================

CREATE OR REPLACE FUNCTION public.cap_nhat_rank_sau_tran()
RETURNS trigger
LANGUAGE plpgsql
SECURITY DEFINER
SET search_path TO 'public'
AS $function$
BEGIN
    IF NEW.result = 'X_WIN' THEN

        UPDATE public.profiles
        SET rank_points = rank_points + 20,
            total_matches = total_matches + 1,
            total_wins = total_wins + 1
        WHERE id = NEW.player_x;

        UPDATE public.profiles
        SET rank_points = rank_points - 10,
            total_matches = total_matches + 1,
            total_losses = total_losses + 1
        WHERE id = NEW.player_o;

    ELSIF NEW.result = 'O_WIN' THEN

        UPDATE public.profiles
        SET rank_points = rank_points - 10,
            total_matches = total_matches + 1,
            total_losses = total_losses + 1
        WHERE id = NEW.player_x;

        UPDATE public.profiles
        SET rank_points = rank_points + 20,
            total_matches = total_matches + 1,
            total_wins = total_wins + 1
        WHERE id = NEW.player_o;

    ELSIF NEW.result = 'DRAW' THEN

        UPDATE public.profiles
        SET rank_points = rank_points + 5,
            total_matches = total_matches + 1,
            total_draws = total_draws + 1
        WHERE id = NEW.player_x;

        UPDATE public.profiles
        SET rank_points = rank_points + 5,
            total_matches = total_matches + 1,
            total_draws = total_draws + 1
        WHERE id = NEW.player_o;

    ELSE
        RAISE EXCEPTION 'Ket qua tran dau khong hop le';
    END IF;

    RETURN NEW;
END;
$function$;


-- =====================================================
-- 7. FUNCTION JOIN CARO ROOM
-- =====================================================

CREATE OR REPLACE FUNCTION public.join_caro_room(
    p_room_code text
)
RETURNS SETOF public.caro_rooms
LANGUAGE plpgsql
SECURITY DEFINER
SET search_path TO 'public'
AS $function$
DECLARE
    v_user uuid;
    v_room public.caro_rooms;
BEGIN
    v_user := auth.uid();

    IF v_user IS NULL THEN
        RAISE EXCEPTION 'Vui long dang nhap';
    END IF;

    UPDATE public.caro_rooms
    SET guest_id = v_user,
        status = 'playing',
        updated_at = now()
    WHERE room_code = upper(trim(p_room_code))
      AND status = 'waiting'
      AND guest_id IS NULL
      AND host_id <> v_user
    RETURNING * INTO v_room;

    IF NOT FOUND THEN

        SELECT *
        INTO v_room
        FROM public.caro_rooms
        WHERE room_code = upper(trim(p_room_code))
          AND (
              host_id = v_user
              OR guest_id = v_user
          );

        IF NOT FOUND THEN
            RAISE EXCEPTION 'Phong khong ton tai hoac da day';
        END IF;

    END IF;

    RETURN NEXT v_room;
END;
$function$;


-- =====================================================
-- 8. FUNCTION LUU TRAN AI
-- =====================================================

CREATE OR REPLACE FUNCTION public.luu_tran_ai(
    p_mode text,
    p_result text
)
RETURNS bigint
LANGUAGE plpgsql
SECURITY DEFINER
SET search_path TO 'public'
AS $function$
DECLARE
    v_user_id UUID;
    v_match_id BIGINT;
    v_diem INTEGER;
BEGIN

    v_user_id := auth.uid();

    IF v_user_id IS NULL THEN
        RAISE EXCEPTION 'Ban chua dang nhap!';
    END IF;

    IF p_mode NOT IN ('AI_KHA', 'AI_KHO') THEN
        RAISE EXCEPTION 'Che do choi khong hop le!';
    END IF;

    IF p_result = 'X_WIN' THEN
        v_diem := 20;

    ELSIF p_result = 'O_WIN' THEN
        v_diem := -10;

    ELSIF p_result = 'DRAW' THEN
        v_diem := 5;

    ELSE
        RAISE EXCEPTION 'Ket qua khong hop le!';
    END IF;

    INSERT INTO public.matches (
        player_x,
        player_o,
        winner_id,
        result,
        mode,
        played_at
    )
    VALUES (
        v_user_id,
        NULL,
        CASE
            WHEN p_result = 'X_WIN' THEN v_user_id
            ELSE NULL
        END,
        p_result,
        p_mode,
        NOW()
    )
    RETURNING id INTO v_match_id;

    UPDATE public.profiles
    SET
        rank_points = COALESCE(rank_points, 0) + v_diem,

        total_matches = COALESCE(total_matches, 0) + 1,

        total_wins = COALESCE(total_wins, 0)
            + CASE WHEN p_result = 'X_WIN' THEN 1 ELSE 0 END,

        total_losses = COALESCE(total_losses, 0)
            + CASE WHEN p_result = 'O_WIN' THEN 1 ELSE 0 END,

        total_draws = COALESCE(total_draws, 0)
            + CASE WHEN p_result = 'DRAW' THEN 1 ELSE 0 END

    WHERE id = v_user_id;

    IF NOT FOUND THEN
        RAISE EXCEPTION 'Khong tim thay ho so nguoi choi!';
    END IF;

    RETURN v_match_id;
END;
$function$;


-- =====================================================
-- 9. FUNCTION MAKE CARO MOVE
-- Xu ly nuoc danh trong phong online
-- =====================================================

CREATE OR REPLACE FUNCTION public.make_caro_move(
    p_room_code text,
    p_row integer,
    p_col integer
)
RETURNS public.caro_rooms
LANGUAGE plpgsql
SECURITY DEFINER
SET search_path TO 'public'
AS $function$
DECLARE
    v_user uuid;
    v_room public.caro_rooms;
    v_board jsonb;
    v_mark text;
    v_winner uuid;
    v_result text;
    v_status text;
    v_count integer;
    v_r integer;
    v_c integer;
    v_dr integer;
    v_dc integer;
    v_k integer;
    v_dirs integer[][] := ARRAY[
        [0,1], [1,0], [1,1], [1,-1]
    ];
    v_d integer;
    v_draw boolean := true;

BEGIN

    v_user := auth.uid();

    IF v_user IS NULL THEN
        RAISE EXCEPTION 'Vui long dang nhap';
    END IF;

    IF p_row < 0 OR p_row >= 15
       OR p_col < 0 OR p_col >= 15 THEN
        RAISE EXCEPTION 'Vi tri khong hop le';
    END IF;

    SELECT *
    INTO v_room
    FROM public.caro_rooms
    WHERE room_code = upper(trim(p_room_code))
    FOR UPDATE;

    IF NOT FOUND THEN
        RAISE EXCEPTION 'Phong khong ton tai';
    END IF;

    IF v_room.status <> 'playing' THEN
        RAISE EXCEPTION 'Tran dau chua san sang hoac da ket thuc';
    END IF;

    IF v_user = v_room.host_id THEN
        v_mark := 'X';

    ELSIF v_user = v_room.guest_id THEN
        v_mark := 'O';

    ELSE
        RAISE EXCEPTION 'Ban khong thuoc phong nay';
    END IF;

    IF v_room.current_turn <> v_mark THEN
        RAISE EXCEPTION 'Chua den luot cua ban';
    END IF;

    v_board := v_room.board;

    -- Khoi tao ban co 15 x 15 neu ban co rong/sai kich thuoc
    IF v_board IS NULL
       OR jsonb_typeof(v_board) <> 'array'
       OR jsonb_array_length(v_board) <> 15 THEN

        SELECT jsonb_agg(row_data ORDER BY r)
        INTO v_board
        FROM (
            SELECT
                r,
                (
                    SELECT jsonb_agg(
                        to_jsonb(''::text)
                        ORDER BY c
                    )
                    FROM generate_series(0,14) c
                ) AS row_data
            FROM generate_series(0,14) r
        ) s;

    END IF;

    IF jsonb_typeof(v_board -> p_row) <> 'array'
       OR jsonb_array_length(v_board -> p_row) <> 15 THEN
        RAISE EXCEPTION 'Du lieu ban co khong hop le';
    END IF;

    IF v_board #>> ARRAY[p_row::text, p_col::text] <> '' THEN
        RAISE EXCEPTION 'O co da duoc danh';
    END IF;

    v_board := jsonb_set(
        v_board,
        ARRAY[p_row::text, p_col::text],
        to_jsonb(v_mark),
        true
    );

    v_winner := NULL;
    v_result := NULL;
    v_status := 'playing';

    -- Kiem tra thang theo 4 huong
    FOR v_d IN 1..4 LOOP

        v_dr := v_dirs[v_d][1];
        v_dc := v_dirs[v_d][2];
        v_count := 1;

        FOR v_k IN 1..4 LOOP

            v_r := p_row + v_dr * v_k;
            v_c := p_col + v_dc * v_k;

            EXIT WHEN v_r < 0 OR v_r >= 15
                OR v_c < 0 OR v_c >= 15;

            EXIT WHEN v_board #>> ARRAY[
                v_r::text, v_c::text
            ] <> v_mark;

            v_count := v_count + 1;

        END LOOP;

        FOR v_k IN 1..4 LOOP

            v_r := p_row - v_dr * v_k;
            v_c := p_col - v_dc * v_k;

            EXIT WHEN v_r < 0 OR v_r >= 15
                OR v_c < 0 OR v_c >= 15;

            EXIT WHEN v_board #>> ARRAY[
                v_r::text, v_c::text
            ] <> v_mark;

            v_count := v_count + 1;

        END LOOP;

        IF v_count >= 5 THEN

            v_winner := CASE
                WHEN v_mark = 'X' THEN v_room.host_id
                ELSE v_room.guest_id
            END;

            v_result := v_mark || '_WIN';
            v_status := 'finished';

            EXIT;

        END IF;

    END LOOP;

    -- Kiem tra hoa
    IF v_status <> 'finished' THEN

        FOR v_r IN 0..14 LOOP

            FOR v_c IN 0..14 LOOP

                IF v_board #>> ARRAY[
                    v_r::text, v_c::text
                ] = '' THEN

                    v_draw := false;
                    EXIT;

                END IF;

            END LOOP;

            EXIT WHEN NOT v_draw;

        END LOOP;

        IF v_draw THEN
            v_result := 'DRAW';
            v_status := 'finished';
        END IF;

    END IF;

    -- Cap nhat phong
    UPDATE public.caro_rooms
    SET
        board = v_board,

        current_turn = CASE
            WHEN v_mark = 'X' THEN 'O'
            ELSE 'X'
        END,

        status = v_status,
        result = v_result,
        winner_id = v_winner,
        updated_at = now()

    WHERE id = v_room.id
    RETURNING * INTO v_room;

    -- Luu lich su neu tran ket thuc
    IF v_status = 'finished' THEN

        INSERT INTO public.matches (
            player_x,
            player_o,
            winner_id,
            result,
            mode
        )
        VALUES (
            v_room.host_id,
            v_room.guest_id,
            v_winner,
            v_result,
            'PVP'
        );

    END IF;

    RETURN v_room;

END;
$function$;


-- =====================================================
-- 10. TRIGGERS
-- =====================================================

DROP TRIGGER IF EXISTS on_auth_user_created ON auth.users;

CREATE TRIGGER on_auth_user_created
AFTER INSERT ON auth.users
FOR EACH ROW
EXECUTE FUNCTION public.handle_new_user();


DROP TRIGGER IF EXISTS trigger_cap_nhat_rank ON public.matches;

CREATE TRIGGER trigger_cap_nhat_rank
AFTER INSERT ON public.matches
FOR EACH ROW
EXECUTE FUNCTION public.cap_nhat_rank_sau_tran();


-- =====================================================
-- 11. ENABLE ROW LEVEL SECURITY
-- =====================================================

ALTER TABLE public.profiles ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.matches ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.caro_rooms ENABLE ROW LEVEL SECURITY;


-- =====================================================
-- 12. RLS POLICIES - MATCHES
-- =====================================================

DROP POLICY IF EXISTS
"Players can view their own matches"
ON public.matches;

CREATE POLICY "Players can view their own matches"
ON public.matches
FOR SELECT
TO authenticated
USING (
    (SELECT auth.uid()) = player_x
    OR (SELECT auth.uid()) = player_o
);


DROP POLICY IF EXISTS player_x_insert_match
ON public.matches;

CREATE POLICY player_x_insert_match
ON public.matches
FOR INSERT
TO authenticated
WITH CHECK (
    auth.uid() = player_x

    AND result = ANY (
        ARRAY['X_WIN', 'O_WIN', 'DRAW']::text[]
    )

    AND (
        winner_id IS NULL
        OR winner_id = player_x
        OR (
            player_o IS NOT NULL
            AND winner_id = player_o
        )
    )

    AND (
        (
            mode = ANY (
                ARRAY['AI_KHA', 'AI_KHO']::text[]
            )
            AND player_o IS NULL
        )
        OR
        (
            mode = '2_PLAYER'
            AND player_o IS NOT NULL
        )
    )
);


-- =====================================================
-- 13. RLS POLICIES - CARO ROOMS
-- =====================================================

DROP POLICY IF EXISTS
"logged in users can create rooms"
ON public.caro_rooms;

CREATE POLICY "logged in users can create rooms"
ON public.caro_rooms
FOR INSERT
TO authenticated
WITH CHECK (
    auth.uid() = host_id
);


DROP POLICY IF EXISTS
"room members can update rooms"
ON public.caro_rooms;

CREATE POLICY "room members can update rooms"
ON public.caro_rooms
FOR UPDATE
TO authenticated
USING (
    auth.uid() = host_id
    OR auth.uid() = guest_id
)
WITH CHECK (
    auth.uid() = host_id
    OR auth.uid() = guest_id
);


DROP POLICY IF EXISTS
"room members can view rooms"
ON public.caro_rooms;

CREATE POLICY "room members can view rooms"
ON public.caro_rooms
FOR SELECT
TO authenticated
USING (
    auth.uid() = host_id
    OR auth.uid() = guest_id
    OR status = 'waiting'
);


-- =====================================================
-- 14. RLS POLICIES - PROFILES
-- =====================================================

DROP POLICY IF EXISTS
"Anyone can view leaderboard"
ON public.profiles;

CREATE POLICY "Anyone can view leaderboard"
ON public.profiles
FOR SELECT
TO anon, authenticated
USING (true);


DROP POLICY IF EXISTS web_profiles_insert
ON public.profiles;

CREATE POLICY web_profiles_insert
ON public.profiles
FOR INSERT
TO authenticated
WITH CHECK (
    auth.uid() = id
);


DROP POLICY IF EXISTS web_profiles_select
ON public.profiles;

CREATE POLICY web_profiles_select
ON public.profiles
FOR SELECT
TO authenticated
USING (
    auth.uid() = id
);


DROP POLICY IF EXISTS web_profiles_update
ON public.profiles;

CREATE POLICY web_profiles_update
ON public.profiles
FOR UPDATE
TO authenticated
USING (
    auth.uid() = id
)
WITH CHECK (
    auth.uid() = id
);


-- =====================================================
-- 15. REALTIME
-- Them caro_rooms vao supabase_realtime neu chua co
-- =====================================================

DO $$
BEGIN

    IF NOT EXISTS (
        SELECT 1
        FROM pg_publication_tables
        WHERE pubname = 'supabase_realtime'
          AND schemaname = 'public'
          AND tablename = 'caro_rooms'
    ) THEN

        ALTER PUBLICATION supabase_realtime
        ADD TABLE public.caro_rooms;

    END IF;

END
$$;


-- =====================================================
-- 16. KIEM TRA DU LIEU
-- =====================================================

-- SELECT * FROM public.profiles;
-- SELECT * FROM public.matches ORDER BY played_at DESC;
-- SELECT * FROM public.caro_rooms ORDER BY created_at DESC;

-- =====================================================
-- END DATABASE SQL
-- =====================================================
