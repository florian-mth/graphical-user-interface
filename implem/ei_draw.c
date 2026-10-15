#include "ei_draw.h"
#include "ei_implementation.h"
#include "intersection.h"
void ei_fill(ei_surface_t surface, const ei_color_t* color, const ei_rect_t* clipper)
{
    ei_rect_t rect = clipper ? *clipper : hw_surface_get_rect(surface);
    ei_rect_t srect = hw_surface_get_rect(surface);

    ei_color_t c = color ? *color : (ei_color_t){0, 0, 0, 255};
    ei_size_t size = hw_surface_get_size(surface);
    uint8_t* buffer = hw_surface_get_buffer(surface);
    int r, g, b, a;
    hw_surface_get_channel_indices(surface, &r, &g, &b, &a);

//coin gauche(x0,y0) et coin droit(x1,y1) du clip

    int x0 = rect.top_left.x ;
    int y0 = rect.top_left.y;
    int x1 = rect.top_left.x + rect.size.width;
    int y1 = rect.top_left.y + rect.size.height;

// Intersection surface - clipper : on force les bornes du clip à rester dans la surface
    
    if (x0 < srect.top_left.x)
    {
        x0 = srect.top_left.x;
    }
    if (y0 < srect.top_left.y)
    {
        y0 = srect.top_left.y;
    }
    if (x1 > srect.top_left.x + srect.size.width)
    {
        x1 = srect.top_left.x + srect.size.width;
    }
    if (y1 > srect.top_left.y + srect.size.height)
    {
        y1 = srect.top_left.y + srect.size.height;
    }

    if (x0 >= x1 || y0 >= y1) return;

    if (c.alpha == 255) {
        uint32_t pixel = ei_impl_map_rgba(surface, c);
        uint32_t* row_buffer = (uint32_t*)buffer;
        int nb_pixels_ligne = x1 - x0;
        for (int y = y0; y < y1; y++) {
            uint32_t* row = row_buffer + y * size.width + x0;
            for (int i = 0; i < nb_pixels_ligne; i++) {
                row[i] = pixel;
            }
        }
        return;
    }

    // widget complètement transparent
    if (c.alpha == 0) {
        return;
    }

    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            uint8_t* dst_pixel = buffer + 4 * (y * size.width + x);

            dst_pixel[r] = (uint8_t)((c.red * c.alpha + dst_pixel[r] * (255 - c.alpha)) / 255);
            dst_pixel[g] = (uint8_t)((c.green * c.alpha + dst_pixel[g] * (255 - c.alpha)) / 255);
            dst_pixel[b] = (uint8_t)((c.blue * c.alpha + dst_pixel[b] * (255 - c.alpha)) / 255);

            if (a >= 0) {
                dst_pixel[a] = 255;
            }
        }
    }
}
int	ei_copy_surface		(ei_surface_t		destination,
                 const ei_rect_t*	dst_rect,
                 ei_surface_t		source,
                 const ei_rect_t*	src_rect,
                 bool			alpha) {
    if (destination == NULL || source == NULL)
        return 1;

    ei_rect_t dst = dst_rect ? *dst_rect : hw_surface_get_rect(destination); //verif si dst_rect est init si oui on prend sinon on prend le rect de destination
    ei_rect_t src= src_rect ? *src_rect : hw_surface_get_rect(source); // pareil mais pour source

    ei_rect_t dst_surface_rect= hw_surface_get_rect(destination); // on recup le rect de destination
    ei_rect_t src_surface_rect= hw_surface_get_rect(source); // on recup le rect de source

    ei_size_t dst_size = hw_surface_get_size(destination); // la size de dest
    ei_size_t src_size = hw_surface_get_size(source); // la size de source

    int dr, dg, db, da; // rgba de la dest
    int sr, sg, sb, sa; // rgba de la source

    if (dst.size.width != src.size.width || dst.size.height != src.size.height) // avant le clipping on regarde si ils ont la meme taille
        return 1;
    hw_surface_get_channel_indices(destination,&dr,&dg,&db,&da); //on recupere les valeurs rgba de la surface destination
    hw_surface_get_channel_indices(source,&sr,&sg,&sb,&sa); // on recupere les valeurs rgba de la surface source
    // on prends les buffers
    uint8_t *dst_buffer = hw_surface_get_buffer(destination);
    uint8_t *src_buffer = hw_surface_get_buffer(source);
    // on recupere les x0,y0 de dest(en haut a gauche) et x1(coins haut droit) et y1(coins bas gauche)
    int dx0 = dst.top_left.x;
    int dy0 = dst.top_left.y;
    int dx1 = dst.top_left.x + dst.size.width;
    int dy1 = dst.top_left.y + dst.size.height;
    // on recupere les x0,y0 de dest(en haut a gauche). Pas besoins de sx1 et et sy1 car on a pas besoin de les modifier dans le cas de clipping. On va juste s'arreter de dessiner avant
    int sx0 = src.top_left.x;
    int sy0 = src.top_left.y;
    // on clipp
    if (dx0< dst_surface_rect.top_left.x) {
        sx0 = dst_surface_rect.top_left.x -dx0;
        dx0 = dst_surface_rect.top_left.x;
    }
    if (dy0< dst_surface_rect.top_left.y) {
        sy0 = dst_surface_rect.top_left.y -dy0;
        dy0 = dst_surface_rect.top_left.y;
    }
    if (dx1 > dst_surface_rect.top_left.x + dst_surface_rect.size.width) {
        dx1 = dst_surface_rect.top_left.x + dst_surface_rect.size.width;
    }
    if (dy1 > dst_surface_rect.top_left.y + dst_surface_rect.size.height) {
        dy1 = dst_surface_rect.top_left.y + dst_surface_rect.size.height;
    }

    if (sx0 < src_surface_rect.top_left.x) {
        dx0 += src_surface_rect.top_left.x - sx0;
        sx0= src_surface_rect.top_left.x;
    }
    if (sy0< src_surface_rect.top_left.y) {
        dy0+= src_surface_rect.top_left.y - sy0;
        sy0= src_surface_rect.top_left.y;
    }

    if (dx0>=dx1 || dy0>=dy1) {
        return 0;
    }

    // on copie les pixels de la src dans la destination
    for (int y =0;y<dy1-dy0;y++) {
        for (int x = 0; x<dx1-dx0;x++) {
            int dst_x = dx0 + x;
            int dst_y = dy0 + y;
            int src_x = sx0 + x;
            int src_y = sy0 + y;

            uint8_t* dst_pixel = dst_buffer + 4*(dst_y*dst_size.width + dst_x); // 4 car rgba transforme en nombre d'octet, (dst_y*dst_size.width + dst_x) trouve le numero du pixel dans le tab global
            uint8_t* src_pixel = src_buffer + 4*(src_y*src_size.width + src_x);

            if (!alpha) {
                //on copie
                dst_pixel[dr] = src_pixel[sr];
                dst_pixel[dg] = src_pixel[sg];
                dst_pixel[db] = src_pixel[sb];
                if (da >=0 && sa >=0) {
                    dst_pixel[da]= src_pixel[sa];
                }
            }else {
                uint8_t src_alpha = (sa >=0) ? src_pixel[sa] : 255;
                // on utilise la formule du diapo (DR*(255 - Sa) + SR*Sa)/255
                dst_pixel[dr]= (uint8_t)((src_pixel[sr]*src_alpha+dst_pixel[dr]*(255-src_alpha))/255);
                dst_pixel[dg]= (uint8_t)((src_pixel[sg]*src_alpha+dst_pixel[dg]*(255-src_alpha))/255);
                dst_pixel[db]= (uint8_t)((src_pixel[sb]*src_alpha+dst_pixel[db]*(255-src_alpha))/255);

                if (da>=0) {
                    dst_pixel[da]= 255;
                }
            }
        }
    }
    return 0;
}

void	ei_draw_text		(ei_surface_t		surface,
                 const ei_point_t*	where,
                 ei_const_string_t	text,
                 ei_font_t		font,
                 ei_color_t		color,
                 const ei_rect_t*	clipper) {

    ei_font_t vrai_font = (font != NULL) ? font : ei_default_font; // si la font est null on prends celle de base
    if (surface == NULL || where == NULL || text == NULL || text[0] == '\0') // si la surface, la destination ou le text est null ou vide on fait rien
        return;
    ei_surface_t text_surface= hw_text_create_surface(text,vrai_font,color); // crée la surface
    if (text_surface == NULL) // en cas d'erreur
        return;
    ei_rect_t dst_rect;
    dst_rect.top_left = *where; // coin haut gauche vaut là où on veut mettre le texte
    dst_rect.size = hw_surface_get_size(text_surface); // sa taille vaut la taille de la surface du texte
    ei_rect_t unclipped_dst_rect = dst_rect;
    ei_rect_t src_rect = {{0, 0}, dst_rect.size};

    if (clipper != NULL) {
        dst_rect = ei_rect_intersection(&dst_rect, clipper); // on fait l'intersection entre le clipper et la dest

        ei_size_t rsz = hw_surface_get_size(surface);
        ei_rect_t rectangle_surface ={.top_left ={0,0}, .size = rsz};
        dst_rect = ei_rect_intersection(&dst_rect, &rectangle_surface); // on clip entre la dst et la surface dessinable

        if (dst_rect.size.width <= 0 || dst_rect.size.height <= 0) { // si on ne peut pas écrire, on free
            hw_surface_free(text_surface);
            return;
        }
        src_rect.top_left.x = dst_rect.top_left.x - unclipped_dst_rect.top_left.x; // on redimensionne la src
        src_rect.top_left.y = dst_rect.top_left.y - unclipped_dst_rect.top_left.y;
        src_rect.size = dst_rect.size;
    }

    hw_surface_lock(text_surface);
    ei_copy_surface(surface, &dst_rect, text_surface, &src_rect, true);
    hw_surface_unlock(text_surface);

    hw_surface_free(text_surface);
}


/* === Ancienne Version de ei_fill === */

// void ei_fill(ei_surface_t surface, const ei_color_t* color, const ei_rect_t* clipper){
//     ei_rect_t rect = clipper ? *clipper : hw_surface_get_rect(surface);
//     ei_color_t c = color ? *color : (ei_color_t){0, 0, 0, 255};
//     uint32_t pixel = ei_impl_map_rgba(surface, c);
//     uint32_t* buffer = (uint32_t*)hw_surace_get_buffer(surface);
//     ei_size_t size = hw_surface_get_size(surface);
//
//     for (int y = rect.top_left.y; y < rect.top_left.y + rect.size.height; y++) {
//         for (int x = rect.top_left.x; x < rect.top_left.x + rect.size.width; x++) {
//             if (x >= 0 && y >= 0 && x < size.width && y < size.height)
//                 buffer[y * size.width + x] = pixel;
//         }
//     }
// }
