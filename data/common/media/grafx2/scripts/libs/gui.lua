--
-- Event-driven GUI library
--
--

gui = {

--
-- dialog() --
--
dialog = function(args)
  local dia = {
    title = args.title,
    w = args.w,
    h = args.h,
    -- 
    widgets = {},
    -- an indexed array, starting at 1. Used for calling the relevant
    -- callback when a numbered control is clicked.  
    callbacks = {},
    
    --
    -- dialog.run() --
    --
    run = function(this)
      windowopen(this.w,this.h, this.title or "");
      -- examine all elements
      for _,widget in ipairs(this.widgets) do
        widget:render()
      end
      
      repeat
       local button, button2, key = windowdodialog();
      
        if button > 0 then
          local c = this.callbacks[button]
          -- run the callback and stop the form if it returns true
          if c ~= nil and c(this) then
              break;
          end
        end
      until key == 27;
      windowclose();
    end
  }
  local id = 1;
  -- examine all elements
  for _,value in ipairs(args) do
    -- all arguments that are tables are assumed to be widgets
    if type(value)=="table" then
      table.insert(dia.widgets, value)
      -- clickable widgets take up an auto-numbered id
      if (value.click) then
        dia.callbacks[id] = value.click
        id=id+1
      end
    end
  end
  return dia;
end,

--
-- button() --
--
button = function(args)
  local but = {
    x = args.x,
    y = args.y,
    w = args.w,
    h = args.h,
    key = args.key,
    label = args.label,
    click = args.click or donothing,
    render = args.repeatable and function(this)
      windowrepeatbutton(this.x, this.y, this.w, this.h, this.label, this.key or -1);
    end
    or function(this)
      windowbutton(this.x, this.y, this.w, this.h, this.label, this.key or -1);
    end
  }
  return but;
end,

--
-- label() --
--
label = function(args)
  local fld = {
    x = args.x,
    y = args.y,
    value = args.value,
    format = args.format,
    fg = args.fg or 0,
    bg = args.bg or 2,
    render = function(this)
      if type(this.format) then
        windowprint(this.x, this.y, string.format(this.format, this.value), this.fg, this.bg);
      else
        windowprint(this.x, this.y, this.value, this.fg, this.bg);
      end
    end
  }
  return fld;
end,


-- "do nothing" function. Used as default callback
donothing = function(this)
end

}
