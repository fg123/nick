#include "layout.h"
#include "util.h"
#include "libpdf/hpdf.h"
#include <stdbool.h>
#include <string.h>

static bool is_viewgroup(view* v) {
    return v->type == TYPE_LINEAR_LAYOUT || v->type == TYPE_FRAME_LAYOUT;
}

static bool is_view(view* v) {
    return !is_viewgroup(v);
}

static float get_full_width(view* v) {
    return v->layout.width + v->layout.margin_left +
    v->layout.margin_right + v->layout.border_left + v->layout.border_right;
}

static float get_full_height(view* v) {
    return v->layout.height + v->layout.margin_top +
    v->layout.margin_bottom + v->layout.border_top + v->layout.border_bottom;
}

static void measure(view* v, float max_width, float max_height) {
    double desired_height = 0;
    double desired_width = 0;
    double width_margin = v->layout.margin_left + v->layout.margin_right;
    double height_margin = v->layout.margin_top + v->layout.margin_bottom;
    double width_padding = v->layout.padding_left + v->layout.padding_right;
    double height_padding = v->layout.padding_top + v->layout.padding_bottom;
    max_width -= width_margin;
    max_height -= height_margin;

    if (v->type == TYPE_TEXT_VIEW) {
        int lines = 1;
        char* text = v->properties.text_view.text;
        HPDF_Rect bbox = HPDF_Font_GetBBox(v->properties.text_view.font);
        // Count Newlines
        char* t = text;
        while (*t) {
            if (t[0] == '\n') lines++;
            t++;
        }

        char* line;
        char* copied_string = strdup(text);
        int index = 0;
        desired_width = 0;
        line = strtok(copied_string, "\n");

        while (line) {
            int max_char =
            HPDF_Font_MeasureText(v->properties.text_view.font,
                text + index,
                strlen(text + index),
                max_width,
                v->properties.text_view.size,
                0, 0, HPDF_TRUE, NULL);
            //printf("Line: %s\n", line);
            int length = strlen(line);
            while (length > max_char) {
                if (max_char == 0) {
                    printf("Fuck %s %f\n", text, max_width);
                    exit(1);
                }
                // Does not fit! Take the first max char items and pull back
                //   until we find a thing to break on. Also will definitely
                //   need all the space we can get.
                desired_width = max_width;
                // Find first thing we can break on and move index there
                //   as well as advancing the line counter.
                int diff = 0;
                int oldI = index;
                int end = max_char;
                for (int i = end; i >= 0; i--) {
                    // printf("%d %c", i, text[i + index]);
                    if (i == 0) {
                        // All the way back at the front.
                        lines++;
                        index += end;
                        //insert_newline(&text, index++);
                        length -= end;
                        break;
                    }
                    else if (text[i + index] == '-' || text[i + index] == ' ') {
                        // Index is now advanced to location after. We trimmed off
                        //   (index + i + 1) - index = i + 1 characters
                        lines++;
                        index += i + 1;
                        //insert_newline(&text, index++);
                        length -= i - 1;
                        break;
                    }
                }
                //printf("\n\n");
                max_char =
                    HPDF_Font_MeasureText(v->properties.text_view.font,
                        text + index,
                        length,
                        max_width,
                        v->properties.text_view.size,
                        0, 0, HPDF_FALSE, NULL);
                //printf("lines: %d\n", lines);
            }
            // If the entire line fits! We good, advance index to after the
            //   newline!
            index += length + 1;
            if (desired_width < max_width) {
                float width;
                HPDF_Font_MeasureText(v->properties.text_view.font,
                    line,
                    strlen(line),
                    max_width,
                    v->properties.text_view.size,
                    0, 0, HPDF_FALSE, &width);
                width = (int)(width + 1);
                desired_width = width > desired_width ? width : desired_width;
            }
            line = strtok(NULL, "\n");
        }
        v->properties.text_view.text = text;
        free(copied_string);
        desired_height = ((int)(bbox.top - bbox.bottom));
        desired_height = (desired_height * v->properties.text_view.size) / 1000.0;
        desired_height *= lines;
    }
    else if (v->type == TYPE_IMAGE_VIEW) {

    }
    else if (v->type == TYPE_LINEAR_LAYOUT) {
        viewlist* child = v->properties.linear_layout.children;
        float used_x = 0;
        float used_y = 0;
        while (child) {
            measure(child->elem, max_width - used_x,
                max_height - used_y);
            if (v->properties.linear_layout.orientation ==
                ORIENTATION_VERTICAL) {
                desired_height += get_full_height(child->elem);
                used_y += get_full_height(child->elem);
                desired_width = max(desired_width, get_full_width(child->elem));
            }
            else {
                desired_width += get_full_width(child->elem);
                used_x += get_full_width(child->elem);
                desired_height = max(desired_height, get_full_height(child->elem));
            }
            child = child->next;
        }
    }
    else if (v->type == TYPE_FRAME_LAYOUT) {
        // max of all the children
        viewlist* child = v->properties.linear_layout.children;
        while (child) {
            measure(child->elem, max_width, max_height);
            desired_height = max(desired_height, get_full_height(child->elem));
            desired_width = max(desired_width, get_full_width(child->elem));
            child = child->next;
        }
    }

    if (v->layout.width_type == SIZE_AUTO) {
        v->layout.width = desired_width + width_padding;
    }
    else if (v->layout.width_type == SIZE_FILL) {
        v->layout.width = max_width;
    }

    if (v->layout.height_type == SIZE_AUTO) {
        v->layout.height = desired_height + height_padding;
    }
    else if (v->layout.height == SIZE_FILL) {
        v->layout.height = max_height;
    }

    v->layout.width_type = SIZE_EXACT;
    v->layout.height_type = SIZE_EXACT;
}

static void offset_position(view* v, float x, float y) {
    v->layout.x += x;
    v->layout.y += y;
    if (v->type == TYPE_LINEAR_LAYOUT) {
        viewlist* child = v->properties.linear_layout.children;
        while (child) {
            offset_position(child->elem, x, y);
            child = child->next;
        }
    }
}

static void position(view* v, float x, float y) {
    x += v->layout.margin_left;
    y += v->layout.margin_top;

    v->layout.x = x;
    v->layout.y = y;
    float vertical_padding = v->layout.padding_top + v->layout.padding_bottom;
    float horizontal_padding = v->layout.padding_left + v->layout.padding_right;
    float tlx = x + v->layout.padding_left;
    float tly = y + v->layout.padding_top;
    float old_tlx = x;
    float old_tly = y;

    if (v->type == TYPE_LINEAR_LAYOUT) {
        viewlist* child = v->properties.linear_layout.children;
        float max_child_height = 0.0;
        float max_child_width = 0.0;
        while (child) {
            position(child->elem, tlx, tly);
            if (v->properties.linear_layout.orientation ==
                    ORIENTATION_VERTICAL) {
                tly += get_full_height(child->elem);
                max_child_width = max(max_child_width,
                    get_full_width(child->elem));
            }
            else {
                tlx += get_full_width(child->elem);
                max_child_height = max(max_child_height,
                    get_full_height(child->elem));
            }
            child = child->next;
        }
        // Position Again After
        float total_child_width = tlx - old_tlx;
        float total_child_height = tly - old_tly;

        child = v->properties.linear_layout.children;
        while (child) {
            float child_width = get_full_width(child->elem);
            float child_height = get_full_height(child->elem);
            float offset_x = 0.0;
            float offset_y = 0.0;
            if (v->layout.gravity & GRAVITY_CENTER_HORIZONTAL) {
                if (v->properties.linear_layout.orientation ==
                    ORIENTATION_VERTICAL) {
                    offset_x = (v->layout.width - horizontal_padding - child_width) / 2.0;
                }
                else {
                    offset_x = (v->layout.width - horizontal_padding - max_child_width) / 2.0;
                }
            }
            if (v->layout.gravity & GRAVITY_CENTER_VERTICAL) {
                if (v->properties.linear_layout.orientation ==
                    ORIENTATION_VERTICAL) {
                    offset_y = (v->layout.height - vertical_padding - max_child_height) / 2.0;
                }
                else {
                    offset_y = (v->layout.height - vertical_padding - child_height) / 2.0;
                }

            }
            if (v->layout.gravity & GRAVITY_RIGHT) {
                offset_x = v->layout.width - horizontal_padding - child_width;
            }
            if (v->layout.gravity & GRAVITY_BOTTOM) {
                offset_y = v->layout.height - vertical_padding - child_height;
            }
            offset_position(child->elem, offset_x, offset_y);
            child = child->next;
        }
    }
    else if (v->type == TYPE_FRAME_LAYOUT) {
        viewlist* child = v->properties.linear_layout.children;
        while (child) {
            float child_width = get_full_width(child->elem);
            float child_height = get_full_height(child->elem);
            float offset_x = 0.0;
            float offset_y = 0.0;
            if (v->layout.gravity & GRAVITY_CENTER_HORIZONTAL) {
                offset_x = (v->layout.width - horizontal_padding - child_width) / 2.0;
            }
            if (v->layout.gravity & GRAVITY_CENTER_VERTICAL) {
                offset_y = (v->layout.height - vertical_padding - child_height) / 2.0;
            }
            if (v->layout.gravity & GRAVITY_RIGHT) {
                offset_x = v->layout.width - horizontal_padding - child_width;
            }
            if (v->layout.gravity & GRAVITY_BOTTOM) {
                offset_y = v->layout.height - vertical_padding - child_height;
            }
            position(child->elem, tlx + offset_x, tly + offset_y);
            child = child->next;
        }
    }
}

static void layout(view* v, float max_width, float max_height) {
    // Measure Children and Position Them
    measure(v, max_width, max_height);
    position(v, 0, 0);
}

// takes a view tree and generates width, height, x and y values
void measure_and_layout(view* tree) {
    return layout(tree, (int)(72.0 * 8.5), 72 * 11);
}