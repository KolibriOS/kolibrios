if not HELPERDIR then
    local CONFIG_HELPERDIR = tup.getconfig("HELPERDIR")
    HELPERDIR = (CONFIG_HELPERDIR == "") and (tup.getcwd() .. "/programs/") or CONFIG_HELPERDIR
end
