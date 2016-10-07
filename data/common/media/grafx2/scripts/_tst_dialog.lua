local counter=0;
local printcounter = function ()
  windowprint(10,54, string.format("% .3d", counter));
end;

windowopen(100,150, "Dialogtest");
windowbutton(6, 18, 54, 14, "Close", 27); -- 1, shortcut=ESC
windowrepeatbutton(6, 38, 14, 14, "+");   -- 2
windowrepeatbutton(26, 38, 14, 14, "-");  -- 3
windowbutton(6, 70, 54, 14, "Help");      -- 4
windowinput(6, 88, 10);
printcounter();

repeat
 local button, button2, key = windowdodialog();

  if button == 2 then -- "+"
    counter=counter+1;
    printcounter();
  end
  if button == 3 then -- "-"
    counter=counter-1;
    printcounter();
  end
  if button == 4 then -- "Help"
    messagebox("Help screen");
  end
until key == 27 or button == 1;
windowclose();


-- messagebox(tostring(button) .. " " .. tostring(button2));


---- Open_window(149,118,"Grid");
-- Display_cursor();
-- Hide_cursor();
---- Close_window();
---- Update_window_area(0,0,Window_width, Window_height);
---- clicked_button=Window_clicked_button();
-- 
-- -- standard button
---- Window_set_normal_button(12,92,51,14,"Cancel",0,1,KEY_ESC);  -- 1
-- -- repeatable button (while held)
---- Window_set_repeatable_button(202,43,13,11,"-",0,1,SDLK_LAST); -- 8
-- -- text input
-- Window_set_input_button(29,24,3); -- 3
-- Window_input_content(input_x_button,str);
-- Readline(31,26,str,3,INPUT_TYPE_INTEGER);
-- 
-- -- dropdown
-- Window_set_dropdown_button(216, 158, 84,14,84,"Preset...", 0,0,1,RIGHT_SIDE|LEFT_SIDE,1);
-- Window_dropdown_clear_items(Button);
-- Window_dropdown_add_item(Button,0,"Set");
-- 
-- -- vertical scroller
-- mix_scroller = Window_set_scroller_button(31,20,84,256,1,Main_backups->Pages->Gradients->Range[Current_gradient].Mix);
-- Window_draw_slider(mix_scroller);
-- 
-- -- display
---- Print_in_window(11,26, "X:",MC_Dark,MC_Light);
---- Print_in_window_limited(Button->Pos_X+3+10,Button->Pos_Y+2,Config.Bookmark_label[bookmark_number],8,MC_Black,MC_Light);
-- Window_display_frame_in( 6, 21,110, 52);
-- Window_display_frame(6,17,130,37);
-- Window_rectangle(panel->Pos_X, panel->Pos_Y, panel->Width, panel->Height+1, MC_Light);

