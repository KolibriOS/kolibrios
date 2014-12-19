#include "board.h"
#include "config.h"

__u8 board_need_config = true;

rect base_cell = {0};
tile null_tile = {0};

struct {
    rect    draw;                       // background rect
    rect    cell_map[BOARD_MAP_SIZE];   // background cells array
    tile    tile_map[BOARD_MAP_SIZE];   // tiles array
    __u16   empty_index[BOARD_MAP_SIZE];// empty cells indexes
    __u16   empty_count;                // empty cells count
    __u32   score;
    __u32   highscore;
} board = {0};

// Get tile index for row and column
__u16 board_index(__u16 row, __u16 column) {
    return column + row * BOARD_COUNT;
}

// Get tile position (as point with eow and column) for index
point board_position(__u16 index) {
    point p = {
        .x = index % BOARD_COUNT,
        .y = index / BOARD_COUNT
    };
    return p;
}

// Calculate cell rect for row and column
rect position2cell(point p) {
    rect c = {0};
    c.width = base_cell.width;
    c.height = base_cell.height;
    c.x = board.draw.x + BOARD_SPACING + p.x * (c.width + BOARD_SPACING);
    c.y = board.draw.y + BOARD_SPACING + p.y * (c.height + BOARD_SPACING);
    return c;
}

// Update information about empty cells
void board_update_empty_info();

// Represent tile array as pointers array
void board_to_tempboard(tile* temp[]);

// Fill tile array with tiles from pointers array
void board_from_tempboard(tile* temp[], __u8 forward);

// Move tile inside a pointer array
void tempboard_move_tile(tile* temp[], __u16 from, __u16 to);

// Merge tiles inside a pointer array
void tempboard_merge_tile(tile* temp[], __u16 from, __u16 to);

// Random number generator
__u32 random_u32(__u32 max);

void board_init(rect* r)
{
    // seed for random number generator
    srand(__menuet__getsystemclock());

    __u16 cell_size = (r->width - BOARD_SPACING * (BOARD_COUNT + 1)) / BOARD_COUNT;
    base_cell.width = cell_size;
    base_cell.height = cell_size;

    null_tile.value = 0;
    null_tile.animate = false;
    null_tile.ani_step = ANI_APPEAR_STEP;
    null_tile.merged = false;

    board.score = 0;
    board.draw = *r;

    canvas_init(r);
    canvas_fill(BOARD_BG_COLOR);

    __u16 i = 0;
    for (i = 0; i < BOARD_MAP_SIZE; i++)
    {
        board.cell_map[i] = position2cell(board_position(i));
        board.tile_map[i] = null_tile;
    }

    __u8 loaded = false;
    __u8 empty_config = true;
    if (board_need_config)
    {
        board_need_config = false;
        config_state state = {0};
        loaded = config_load(&state);
        if(loaded)
        {
            board.score = state.score;
            board.highscore = state.highscore;

            i = 0;
            for (i = 0; i < BOARD_MAP_SIZE; i++)
            {
                if (state.value_map[i])
                {
                    empty_config = false;
                    board_add_tile(state.value_map[i],i);
                }
            }
        }
    }

    if (!loaded || empty_config)
    {
        i = 0;
        for (i = 0; i < START_COUNT; i++)
        {
            board_add_random_tile();
        }
    }

    board_redraw();
}

void board_delete()
{
    config_state state = {0};
    state.score = board.score;
    state.highscore = board.highscore;
    int i = 0;
    for (i = 0; i < BOARD_MAP_SIZE; i++)
        state.value_map[i] = board.tile_map[i].value;
    config_save(&state);

    canvas_delete();
}

void board_redraw()
{
    __u16 i = 0;
    __u8 animate = false;
    __u8 last_animate = false;
    do
    {
        canvas_fill(BOARD_BG_COLOR);

        for (i = 0; i < BOARD_MAP_SIZE; i++)
        {
            canvas_draw_rect(&board.cell_map[i],CELL_COLOR);
        }

        animate = false;
        last_animate = false;
        for (i = 0; i < BOARD_MAP_SIZE; i++)
        {
            tile* t = &board.tile_map[i];
            last_animate = tile_draw(t);
            if (last_animate)
            {
                animate = last_animate;
            }
        }

        canvas_paint();

        if (animate)
        {
            __menuet__delay100(ANI_DELAY);
        }
    }
    while (animate);
}

__u8 board_up()
{
    __u8 moved = false;

    __u16 row = 0;
    __u16 column = 0;
    __u16 ind = 0;
    __u16 preind = 0;
    tile* indtile = 0;
    tile* pretile = 0;

    tile* temp_board[BOARD_MAP_SIZE] = {0};
    board_to_tempboard(temp_board);

    for (column = 0; column < BOARD_COUNT; column++)
    {
        for (row = 0; row < BOARD_COUNT; row++)
        {
            if (row > 0)
            {
                ind = board_index(row,column);
                indtile = temp_board[ind];
                if (indtile)
                {
                    preind = board_index(row - 1,column);
                    pretile = temp_board[preind];
                    if (!pretile)
                    {
                        moved = true;
                        tempboard_move_tile(temp_board,ind,preind);
                        row = 0;
                    }
                    else if (tile_mergeable(indtile,pretile))
                    {
                        moved = true;
                        board.score += indtile->value * 2;
                        if (board.score > board.highscore)
                            board.highscore = board.score;
                        tempboard_merge_tile(temp_board,ind,preind); 
                        row = 0;
                    }
                }
            }
        }
    }

    board_from_tempboard(temp_board,true);

    return moved;
}

__u8 board_down()
{
    __u8 moved = false;

    __u16 row = 0;
    __u16 column = 0;
    __u16 ind = 0;
    __u16 preind = 0;
    tile* indtile = 0;
    tile* pretile = 0;

    tile* temp_board[BOARD_MAP_SIZE] = {0};
    board_to_tempboard(temp_board);

    for (column = 0; column < BOARD_COUNT; column++)
    {
        row = BOARD_COUNT;
        while (row--)
        {
            if ((BOARD_COUNT - row) > 1)
            {
                ind = board_index(row,column);
                indtile = temp_board[ind];
                if (indtile)
                {
                    preind = board_index(row + 1,column);
                    pretile = temp_board[preind];
                    if (!pretile)
                    {
                        moved = true;
                        tempboard_move_tile(temp_board,ind,preind);
                        row = BOARD_COUNT;
                    }
                    else if (tile_mergeable(indtile,pretile))
                    {
                        moved = true;
                        board.score += indtile->value * 2;
                        if (board.score > board.highscore)
                            board.highscore = board.score;
                        tempboard_merge_tile(temp_board,ind,preind);
                        row = BOARD_COUNT;
                    }
                }
            }
        }
    }

    board_from_tempboard(temp_board,false);

    return moved;
}

__u8 board_left()
{
    __u8 moved = false;

    __u16 row = 0;
    __u16 column = 0;
    __u16 ind = 0;
    __u16 preind = 0;
    tile* indtile = 0;
    tile* pretile = 0;

    tile* temp_board[BOARD_MAP_SIZE] = {0};
    board_to_tempboard(temp_board);

    for (row = 0; row < BOARD_COUNT; row++)
    {
        for (column = 0; column < BOARD_COUNT; column++)
        {
            if (column > 0)
            {
                ind = board_index(row,column);
                indtile = temp_board[ind];
                if (indtile)
                {
                    preind = board_index(row,column - 1);
                    pretile = temp_board[preind];
                    if (!pretile)
                    {
                        moved = true;
                        tempboard_move_tile(temp_board,ind,preind);
                        column = 0;
                    }
                    else if (tile_mergeable(indtile,pretile))
                    {
                        moved = true;
                        board.score += indtile->value * 2;
                        if (board.score > board.highscore)
                            board.highscore = board.score;
                        tempboard_merge_tile(temp_board,ind,preind);
                        column = 0;
                    }
                }
            }
        }
    }

    board_from_tempboard(temp_board,true);

    return moved;
}

__u8 board_right()
{
    __u8 moved = false;

    __u16 row = 0;
    __u16 column = 0;
    __u16 ind = 0;
    __u16 preind = 0;
    tile* indtile = 0;
    tile* pretile = 0;

    tile* temp_board[BOARD_MAP_SIZE] = {0};
    board_to_tempboard(temp_board);

    for (row = 0; row < BOARD_COUNT; row++)
    {
        column = BOARD_COUNT;
        while (column--)
        {
            if ((BOARD_COUNT - column) > 1)
            {
                ind = board_index(row,column);
                indtile = temp_board[ind];
                if (indtile)
                {
                    preind = board_index(row,column + 1);
                    pretile = temp_board[preind];
                    if (!pretile)
                    {
                        moved = true;
                        tempboard_move_tile(temp_board,ind,preind);
                        column = BOARD_COUNT;
                    }
                    else if (tile_mergeable(indtile,pretile))
                    {
                        moved = true;
                        board.score += indtile->value * 2;
                        if (board.score > board.highscore)
                            board.highscore = board.score;
                        tempboard_merge_tile(temp_board,ind,preind);
                        column = BOARD_COUNT;
                    }
                }
            }
        }
    }

    board_from_tempboard(temp_board,false);

    return moved;
}

__u8 board_add_random_tile()
{
    board_update_empty_info();
    if (board.empty_count)
    {
        __u16 rnd_av = random_u32(board.empty_count);
        rnd_av = board.empty_index[rnd_av];
        __u32 rnd_value = (random_u32(10) < 9) ? 2 : 4;

        board_add_tile(rnd_value,rnd_av);
    }
    return board.empty_count;
}

void board_add_tile(__u32 value, __u16 index)
{
    tile* av_tile = &board.tile_map[index];
    av_tile->value = value;

    av_tile->animate = true;
    av_tile->ani_step = ANI_APPEAR_STEP;
    av_tile->transition = position2cell(board_position(index));
    av_tile->cell.x = av_tile->transition.x + base_cell.width / 2;
    av_tile->cell.y = av_tile->transition.y + base_cell.height / 2;
    av_tile->cell.width = 0;
    av_tile->cell.height = 0;
}

__u8 board_has_moves()
{
    __u16 ind = 0;
    __u16 next = 0;
    __u16 step = 0;
    __u16 pos = 0;
    for (step = 0; step < BOARD_COUNT; step++)
    {
        for (pos = 0; pos < BOARD_COUNT; pos++)
        {
            // check horizontal
            ind = board_index(step,pos);
            next = board_index(step,pos + 1);

            if (!board.tile_map[ind].value ||
                    (((pos + 1) < BOARD_COUNT) &&
                     (!board.tile_map[next].value ||
                      (board.tile_map[ind].value == board.tile_map[next].value)
                      )
                     )
                    )
                return true;

            // check vertical
            ind = board_index(pos,step);
            next = board_index(pos + 1,step);

            if (!board.tile_map[ind].value ||
                    (((pos + 1) < BOARD_COUNT) &&
                     (!board.tile_map[next].value ||
                      (board.tile_map[ind].value == board.tile_map[next].value)
                      )
                     )
                    )
                return true;
        }
    }
    return false;
}

__u32 board_score()
{
    return board.score;
}

__u32 board_highscore()
{
    return board.highscore;
}

void board_update_empty_info()
{
    board.empty_count = 0;

    __u16 i = 0;
    for (i = 0; i < BOARD_MAP_SIZE; i++)
    {
        if (!board.tile_map[i].value)
        {
            board.empty_index[board.empty_count] = i;
            board.empty_count++;
        }
    }
}

void board_to_tempboard(tile* temp[])
{
    __u16 ind = 0;
    for (ind = 0; ind < BOARD_MAP_SIZE; ind++)
    {
        tile* bt = &board.tile_map[ind];
        if (bt->value)
        {
            temp[ind] = bt;
        }
    }
}

void board_from_tempboard(tile *temp[], __u8 forward)
{
    __u16 ind = 0;
    if (forward)
    {
        for (ind = 0; ind < BOARD_MAP_SIZE; ind++)
        {
            tile* bt = &board.tile_map[ind];
            tile* tt = temp[ind];
            if (tt)
            {
                *bt = *tt;
                bt->transition = position2cell(board_position(ind));
            }
            else
            {
                *bt = null_tile;
            }
        }
    }
    else
    {
        ind = BOARD_MAP_SIZE;
        while (ind--)
        {
            tile* bt = &board.tile_map[ind];
            tile* tt = temp[ind];
            if (tt)
            {
                *bt = *tt;
                bt->transition = position2cell(board_position(ind));
            }
            else
            {
                *bt = null_tile;
            }
        }
    }
}

void tempboard_move_tile(tile* temp[], __u16 from, __u16 to)
{
    temp[to] = temp[from];
    temp[to]->animate = true;
    temp[to]->ani_step = ANI_MOVE_STEP;
    temp[from] = 0;
}

void tempboard_merge_tile(tile* temp[], __u16 from, __u16 to)
{
    temp[from]->merged = true;
    temp[from]->merged_rect = temp[to]->cell;
    tempboard_move_tile(temp,from,to);
}

__u32 random_u32(__u32 max)
{
    return ((rand() * 1.0) / RAND_MAX) * max;
}
