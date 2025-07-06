/****************************************************************************
*   raylib [textures] example - Image loading and drawing on it
*   NOTE: Images are loaded in CPU memory (RAM); textures are loaded in GPU memory (VRAM)
*   Example originally created with raylib 1.4, last time updated with raylib 1.4
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*   Copyright (c) 2016-2024 Ramon Santamaria (@raysan5)
****************************************************************************   */

#include "raylib.h"

// Program main entry point
int main ( void ) {
  // -----  setup  -----
  const int WINWIDTH = 800;
  const int WINHEIGHT = 450;

  // .....  hi-res setup  .....
  SetConfigFlags ( FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI );
  InitWindow ( WINWIDTH, WINHEIGHT, "raylib [textures] example - image drawing" );
  // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
  Image cat = LoadImage ( "resources/cat.png" ); // Load image in CPU memory (RAM)

  ImageCrop ( &cat, ( Rectangle ) {
              100, 10, 280, 380} );              // Crop an image piece
  ImageFlipHorizontal ( &cat );                  // Flip cropped image horizontally
  ImageResize ( &cat, 150, 200 );                // Resize flipped-cropped image
  Image parrots = LoadImage ( "resources/parrots.png" );  // Load image in CPU memory (RAM)

  // Draw one image over the other with a scaling of 1.5f
  ImageDraw ( &parrots, cat, ( Rectangle ) {
              0, 0, ( float ) cat.width, ( float ) cat.height}, ( Rectangle ) {
              30, 40, cat.width * 1.5f, cat.height * 1.5f}, WHITE );
  ImageCrop ( &parrots, ( Rectangle ) {
              0, 50, ( float ) parrots.width, ( float ) parrots.height - 100} );  // Crop resulting image
  // Draw on the image with a few image draw methods
  ImageDrawPixel ( &parrots, 10, 10, RAYWHITE );
  ImageDrawCircleLines ( &parrots, 10, 10, 5, RAYWHITE );
  ImageDrawRectangle ( &parrots, 5, 20, 10, 10, RAYWHITE );
  UnloadImage ( cat );                           // Unload image from RAM
  // Load custom font for frawing on image
  Font font = LoadFont ( "resources/custom_jupiter_crash.png" );

  // Draw over image using custom font
  ImageDrawTextEx ( &parrots, font, "PARROTS & CAT", ( Vector2 ) {
                    300, 230}, ( float ) font.baseSize, -2, WHITE );
  UnloadFont ( font );            // Unload custom font (already drawn used on image)
  Texture2D texture = LoadTextureFromImage ( parrots ); // Image to texture, sent to GPU

  UnloadImage ( parrots );        // image now texture and in gpu, can be unloaded from RAM
  SetTargetFPS ( 60 );

  // Main game loop
  while ( !WindowShouldClose (  ) )              // quit if win close btn or ESC key
  {
    // Update
    // TODO: Update your variables here

    // Draw
    BeginDrawing (  );
    ClearBackground ( BLACK );
    DrawTexture ( texture, WINWIDTH / 2 - texture.width / 2,
                  WINHEIGHT / 2 - texture.height / 2 - 40, WHITE );
    DrawRectangleLines ( WINWIDTH / 2 - texture.width / 2,
                         WINHEIGHT / 2 - texture.height / 2 - 40,
                         texture.width, texture.height, GREEN );
    DrawText ( "Drawing only one texture from various images!", 140, 350, 20, LIGHTGRAY );
    DrawText
      ( "Images are cropped, scaled, flipped and overlap.", 140, 370, 20, LIGHTGRAY );
    EndDrawing (  );
  }

  // De-Initialization
  UnloadTexture ( texture );                     // Texture unloading
  CloseWindow (  );                              // Close window and OpenGL context
  return 0;
}