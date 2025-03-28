


/*
void set_bg(ImageId id)
{
    load_image(id);
    if(gs.bg == id )
        return;
    
    free_image(gs.bg);
    gs.bg = id;
}
*/




void hog_init()
{
    //set_bg(TITLE_BG);
    load_image(TITLE_BG);
    
    load_image(HOG_ANI);
    load_font(FONT_0);
    
    Entity bg = {0};
    bg.pos = (V2){400,0};
    bg.type = DECOR_ENT;
    bg.sprite_id = TITLE_BG_SPR;
    add_entity(&bg);
    
    Entity soda = {0};
    soda.pos = (V2){700,200};
    soda.type = COLLECTABLE_ENT;
    soda.sprite_id = SODA_SPR;
    add_entity(&soda);
    
    Entity e = {0};
    e.sprite_id = HOG_RUN_SPR;
    e.pos = (V2){150,200};
    add_entity(&e);
    
    
    
    //testing json loading
    char* level = (char*)read_file("assets/levels/level_0.json");
    u32 level_size = get_file_size("assets/levels/level_0.json");
    
    //printf("%s\n", (char*)level);
    struct json_value_s* json = json_parse(level, level_size);
    struct json_object_s *root = json->payload;
    struct json_object_element_s *test = root->start;
    struct json_string_s *test_string = test->value->payload;
    
    //struct json_object_s* test = json_value_as_object(root)->start;
    
    printf("%s\n", (char*)test_string->string);
    free(json);
    free(level);
}

char *directions = "Press right to get soda";

void hog_update(GameInput* game_input)
{
    gi = game_input;
    gs.total_time += gi->elapsed_seconds;
    pre_render();
    
    
    //draw_entity(&gs.entities[0]);
    
    
    update_entities();
    update_sprites();
    //draw_sprite(300, 300, HOG_RUN_SPR, (i32)frames);
    
    
    //draw_text(30,300, 1.0, FONT_0, "hello there\nIT is SODA TIME!\nYOU HOG!!!!");
    
    draw_entities();
    
    
    draw_text(50,50,1,FONT_0,directions);
    render();
    
    
}


void hog_clean_up()
{
    free_image(TITLE_BG);
    free_image(HOG_ANI);
    free_font(FONT_0);
}