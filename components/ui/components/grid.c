#include <lvgl.h>
#include <stdio.h>
#include <../colors.h>

#define BUTTON_SIZE 70
#define BUTTON_PADDING 16
#define BUTTON_GAP 4

#define GRID_COLS 9
#define GRID_ROWS 5 // Totals 45 buttons per page

static lv_obj_t *grid_buttons[GRID_ROWS][GRID_COLS];

lv_obj_t *create_macro_grid(lv_obj_t *parent)
{
    // Creates the main container for the grid
    lv_obj_t *grid = lv_obj_create(parent);
    lv_obj_remove_style_all(grid); // Removes all default styles from the grid

    lv_coord_t grid_width = (BUTTON_SIZE * GRID_COLS) + (BUTTON_GAP * (GRID_COLS - 1));
    lv_coord_t grid_height = (BUTTON_SIZE * GRID_ROWS) + (BUTTON_GAP * (GRID_ROWS - 1));

    lv_obj_set_size(grid, grid_width, grid_height);
    lv_obj_center(grid);

    for (int row = 0; row < GRID_ROWS; row++)
    {
        for (int col = 0; col < GRID_COLS; col++)
        {
            // Create button
            lv_obj_t *btn = lv_btn_create(grid);
            grid_buttons[row][col] = btn;

            // Set button size
            lv_obj_set_size(btn, BUTTON_SIZE, BUTTON_SIZE);

            // Position button
            lv_coord_t x_pos = col * (BUTTON_SIZE + BUTTON_GAP);
            lv_coord_t y_pos = row * (BUTTON_SIZE + BUTTON_GAP);
            lv_obj_set_pos(btn, x_pos, y_pos);

            // Style button
            lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_SECONDARY), 0);
            lv_obj_set_style_radius(btn, 12, 0); // Rounded corners
            lv_obj_set_style_border_width(btn, 1, 0);
            lv_obj_set_style_border_color(btn, lv_color_hex(COLOR_PRIMARY), 0);

            // Remove focus styling
            lv_obj_add_flag(btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
            lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICK_FOCUSABLE);

            // Add plus icon
            lv_obj_t *label = lv_label_create(btn);
            lv_label_set_text(label, LV_SYMBOL_PLUS);
            lv_obj_set_style_text_color(label, lv_color_hex(COLOR_PRIMARY), 0);
            lv_obj_center(label);

            // Add click event - not used for now.
            // lv_obj_add_event_cb(btn, grid_event_handler, LV_EVENT_CLICKED, NULL);
        }
    }

    return grid;
}