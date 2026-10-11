local off_val = "full"
local on_val = ""

local function to_list(str)
  if type(str) == "string" then
    str = {str}
  end
  return str
end

function to_build(compilers)
  compilers = to_list(compilers)

  for _, c in ipairs(compilers) do
    if tup.getconfig("NO_" .. c) ~= on_val then
      return false
    end
  end

  return true
end

function to_image(compilers)
  compilers = to_list(compilers)

  for _, c in ipairs(compilers) do
    if tup.getconfig("NO_" .. c) == off_val then
      return false
    end
  end

  return true
end

function lang(langs)
  local build_type = tup.getconfig("BUILD_TYPE")
  langs = to_list(langs)

  for _, l in ipairs(langs) do
    if build_type == l then return true end
  end

  return false
end
