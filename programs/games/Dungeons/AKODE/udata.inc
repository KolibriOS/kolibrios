; =========================== Uninitialized data ============================= ;

align 4
akode_data.BlockSize                    akode.Size                      ; first - width and height of the block base, must be power of 2
                                                                        ; second - height of the block
akode_data.BlockWidthPowerOf2           dd ?
akode_data.Camera                       akode.Camera
akode_data.ProjectionPlane              akode.ProjectionPlane

akode_data.CameraToPlaneDistance        dd ?
akode_data.WallHeightDividend           dd ?
akode_data.FloorDistanceDividend        dd ?
akode_data.CeilingDistanceDividend      dd ?

akode_data.Angle90                      dd ?
akode_data.Angle180                     dd ?
akode_data.Angle270                     dd ?
akode_data.Angle360                     dd ?

akode_data.ShadingColor                 dd ?
akode_data.ShadingDistance              dd ?

akode_data.CurrentLevel                 akode.LevelHeader

akode_data.MovementSpeed                dd ?
akode_data.TurningSpeed                 dd ?
akode_data.MovementDirection            dd ?
akode_data.TurningDirection             dd ?
akode_data.LastMoveTimestamp            dd ?
akode_data.LastTurnTimestamp            dd ?

akode_data.ImageBufferSize              dd ?

akode_data.OptimizedGetImage            dd ?