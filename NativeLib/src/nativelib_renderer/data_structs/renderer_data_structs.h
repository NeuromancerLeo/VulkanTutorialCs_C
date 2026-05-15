/// 这里声明着与渲染器进行互操作所需的所有数据结构

#pragma once

#include "vertex_data.h"

#include "data_in_buffer/uniform/camera_uniform.h"
#include "data_in_buffer/uniform/unlit_material_uniform.h"
#include "data_in_buffer/uniform/draw_item_uniform.h"

#include "draw_item_info.h"
#include "indexed_draw_item_info.h"

#include "pipeline_info/pipeline_draw_task.h"
#include "pipeline_info/main_render_pass_draw_infos.h"

#include "pipeline_info/triangle/triangle_pipeline_draw_infos.h"
#include "pipeline_info/unlit/unlit_pipeline_draw_infos.h"