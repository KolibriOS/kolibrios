tup.append_table(tmp_extra_files, {
  {"kolibrios/drivers/drvinf.ini", tup.getcwd() .. "/drvinf.ini"},
  {"kolibrios/drivers/ahci/", tup.getcwd() .. "/ahci/*"},
  {"kolibrios/drivers/acpi/readme.txt", tup.getcwd() .. "/acpi/readme.txt"},
  -- {"kolibrios/drivers/acpi/", tup.getcwd() .. "/acpi/*"},
  {"kolibrios/drivers/atikms/", tup.getcwd() .. "/atikms/*"},
  {"kolibrios/drivers/i915/", tup.getcwd() .. "/i915/*"},
  {"kolibrios/drivers/test/", tup.getcwd() .. "/test/*"},
  {"kolibrios/drivers/vmware/", tup.getcwd() .. "/vmware/*"},
  {"kolibrios/drivers/virtualbox/", tup.getcwd() .. "/virtualbox/*"},
  -- {"kolibrios/drivers/geode/", tup.getcwd() .. "/geode/*"},
})

if to_image("GCC") then
  tup.append_table(tmp_extra_files, {
    {"kolibrios/drivers/geode/geode.sys", tup.getcwd() .. "/geode/geode.sys"}, -- there is also an autobuid version that is not working
  })
end
