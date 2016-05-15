import bpy

if __name__ == "__main__":
    bpy.context.scene.render.engine = "CYCLES"
    bpy.context.scene.render.resolution_x = 1920
    bpy.context.scene.render.resolution_y = 1080
    bpy.context.scene.render.resolution_percentage = 100
    bpy.context.scene.render.use_antialiasing = True
    bpy.context.scene.render.use_full_sample = True
    
    # Set the format
    bpy.data.scenes["Scene"].render.image_settings.file_format = "PNG"
    bpy.data.scenes["Scene"].render.image_settings.color_mode = "RGBA"
    
    # Cycles sampling
    # bpy.data.scenes["Scene"].cycles.samples = 512
    
    # Format output
    bpy.data.scenes["Scene"].render.filepath = "//output/wallpaper.png"
    
    # Render the single frame
    bpy.ops.render.render(write_still = True)
