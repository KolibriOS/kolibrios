kolibri = kolibri or {}

function kolibri.install(spec)
  local lists = {
    img   = tmp_img_files,          -- floppy image
    extra = tmp_extra_files,        -- common extra list
    iso   = tmp_iso_extra_files,    -- for ISO only
    distr = tmp_distr_extra_files,  -- for distribution_kit only
  }

  local list = assert(
    lists[spec.target],
    "Unknown target: " .. tostring(spec.target)
  )

  local dir
  if spec.kind == "built" then       -- the input is compiled
    dir = tup.getvariantdir()
  elseif spec.kind == "source" then  -- the input is not compiled
    dir = tup.getcwd()
  else
    error("Unknown file kind: " .. tostring(spec.kind) ..
      ". Use one of \"built\" or \"source\"")
  end

  table.insert(list, {
    spec.dst,
    dir .. "/" .. spec.src,
  })
end
