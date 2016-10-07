-- Persistence library:
--   Memorize data for current function
--   memory.save(tab) and tab=memory.load()
--
--   The data will be stored in file called
--     <calling_function_name>.dat
--   in the lua directory 
-- 
-- Example 1:
--
--  -- Load initial values or set defaults
--  arg = memory.load({picX=320,picY=200,scale=0})
--  -- Run an inputbox
--  OK,arg.picX,arg.picY,arg.scale = inputbox("Image Size")",
--    "Width",    arg.picX,  1,2048,0,
--    "Height",   arg.picY,  1,2048,0,
--    "Scale",    arg.scale, 0,1,0);
--  if OK == true then
--    -- Save the selected values
--    memory.save(arg)
--  end

-- Example 2:
--
--  -- Load initial values or set defaults
--  arg = memory.load({x=320,y=200,scale=0})
--  picX=arg.x
--  picY=arg.y
--  scale=arg.scale
--  -- Run an inputbox
--  OK,picX,picY,scale = inputbox("Image Size")",
--    "Width",    picX,  1,2048,0,
--    "Height",   picY,  1,2048,0,
--    "Scale",    scale, 0,1,0);
--  if OK == true then
--    -- Save the selected values
--    memory.save({x=picX,y=picY,scale=scale})
--  end

  
memory =
{
  serialize = function(o)
    if type(o) == "number" then
      return tostring(o)
    elseif type(o) == "string" then
      return string.format("%q", o)
    --elseif type(o) == "table" then
    --  io.write("{\n")
    --  for k,v in pairs(o) do
    --    io.write("  ", k, " = ")
    --    memory.serialize(v)
    --    io.write(",\n")
    --  end
    --  io.write("}\n")
    else
      error("cannot serialize a " .. type(o))
    end
  end;
  
  -- Return a string identifying the calling function.
  -- Pass 1 for parent, 2 for grandparent etc.
  callername = function(level)
    local w
    local last_slash
    local info = debug.getinfo(level+1,"Sn")
    local caller=tostring(info.name)
    -- Function name if possible
    if (caller~="nil") then
      return caller
    end
    -- Otherwise, get file name, without extension
   
    -- Get part after directory name
    last_slash=0
    while true do
      local pos = string.find(info.source, "/", last_slash+1)
      if (pos==nil) then break end
      last_slash=pos
    end
    while true do
      local pos = string.find(info.source, "\\", last_slash+1)
      if (pos==nil) then break end
      last_slash=pos
    end
    
    caller=string.sub(info.source, last_slash+1)
    
    -- Remove file extension
    if (string.sub(caller,-4, -1)==".lua") then
      caller=string.sub(caller, 1, -5)
    end
    return caller
  end;
  
  -- Memorize some parameters.
  save = function(o)
    local caller=memory.callername(2)
    --for k, v in pairs(o) do
    --    messagebox(tostring(k))
    --    messagebox(tostring(v))
    --end
    local f, e = io.open(caller..".dat", "w");
    if (f ~= nil) then
      f:write("Entry {\n")
      for k, v in pairs(o) do
        if (type(v)=="number") then
          f:write("  "..k.."="..memory["serialize"](v)..",\n")
        end
      end
      f:write("}\n")
      f:close()
    end  
  end;
  
  
  -- Recover some saved parameters.
  load = function(o)
    local caller=memory.callername(2)
    local i

    function Entry (b)
      -- Adds (or replaces) values in arg with those from b
      for k, v in pairs(b) do
        o[k]=v
      end
    end
    local f = (loadfile(caller..".dat"))
    if (f ~= nil) then
      f()
    end

    return o
  end;

}

return memory
