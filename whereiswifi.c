//
//  https://www.codeeval.com/open_challenges/159/
//  whereiswifi
//
//  Created by Stephan Warren on 12/13/14.
//  Copyright (c) 2014 Stephan Warren. All rights reserved.
//
/*
 
 CHALLENGE DESCRIPTION:
 
 
 The car with the Wi-Fi radar is driving along the streets. It collects information about the accessible Wi-Fi hotspots in some definite places by determining MAC-address and azimuth angle to the hotspot. Also, the city map is available. It provides the list of buildings and coordinates of polygons vertices that form their outlines.
 
 Your task is to determine in which buildings hotspots are located.
 
 INPUT SAMPLE:
 
 The first argument is a path to a file that contains the city map and the Wi-Fi radar log. The city map is separated from the Wi-Fi radar log by an empty line.
 
 The city map is represented as a list of buildings, one building per line. Each line of city map data starts with a building name and is followed by coordinates of vertices that form its outline. Coordinates are pairs X and Y separated by semicolon ‘;’. Coordinates are separated by space.
 
 Each line of the Wi-Fi radar log starts with coordinates X and Y separated by semicolon ‘;’ of radar’s current position. If any hotspot was detected radar coordinates are followed by its MAC-address and azimuth angle (in degrees) separated by semicolon ‘;’.
 
 B001 14.88;8.94 14.88;33.23 25.29;33.23 25.29;15.88 32.23;15.88 32.23;8.94 14.88;8.94
 B002 14.88;33.23 14.88;43.64 49.58;43.64 49.58;26.29 39.17;26.29 39.17;33.23 14.88;33.23
 ... some lines skipped ...
 B010 63.45;50.58 70.39;50.58 70.39;43.64 63.45;43.64 63.45;50.58
 
 56.51;5.47 56-4c-18-eb-13-8b;59.3493 88-fe-14-a4-aa-2a;303.0239
 42.64;5.47 88-fe-14-a4-aa-2a;0.0000
 28.76;5.47 88-fe-14-a4-aa-2a;56.9761
 14.88;5.47 88-fe-14-a4-aa-2a;71.9958
 11.41;15.88
 11.41;29.76
 ... some lines skipped ...
 56.51;64.45 f9-aa-de-15-28-46;277.5946 de-c2-8e-34-08-17;214.6952
 OUTPUT SAMPLE:
 
 Print to stdout names of buildings which have Wi-Fi hotspots. The order should be alphabetical.
 
 B003
 B005
 B007
 CONSTRAINTS:
 
 0° azimuth direction is the same as +Y axis direction
 Hotspots can be located both inside and outside of the building
 There can be more than one hotspot in the building
 Hotspots are immovable during all measurements
 
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>




//#define debug_print(fmt, ...) do { fprintf(stderr, fmt, __VA_ARGS__); } while (0)

#define debug_print(...) do {} while (0)



#define MY_PI 3.14159265358979323846

typedef struct { double x; double y; } coord_t;
typedef struct { char mac[18]; double az; double slope; double offset; char calc; char vertical;} ray_t;
typedef struct { char * name; coord_t * cptr; unsigned short wificnt; unsigned char cnt;} building_t;
typedef struct { coord_t coord; ray_t * ray; unsigned char cnt;} location_t;

//#define JSCRIPT

#ifdef JSCRIPT
FILE * js;
#define XS 800
#define YS XS
#define FACTOR ((float) XS) / 110.0

coord_t ave;

void drawline(coord_t p1, coord_t p2) {
    
    int x1 = (int) (p1.x * FACTOR) + 1;
    int x2 = (int) (p2.x * FACTOR) + 1;
    int y1 = YS - (int) (p1.y  * FACTOR) + 1;
    int y2 = YS - (int) (p2.y  * FACTOR) + 1;
    
    fprintf(js, "    context.moveTo(%d, %d);\n", x1, y1);
    fprintf(js, "    context.lineTo(%d, %d);\n", x2, y2);
    fprintf(js, "    context.stroke();\n");
}

void drawpoint(coord_t p)
{
    int x = (int) (p.x * FACTOR) + 1;
    int y = YS - (int) (p.y  * FACTOR) + 1;
    
    fprintf(js, "    context.moveTo(%d, %d);\n", x+1, y+1);
    fprintf(js, "    context.lineTo(%d, %d);\n", x-1, y-1);
    fprintf(js, "    context.stroke();\n");

    fprintf(js, "    context.moveTo(%d, %d);\n", x-1, y+1);
    fprintf(js, "    context.lineTo(%d, %d);\n", x+1, y-1);
    fprintf(js, "    context.stroke();\n");
    
    
}

void sayBuilding(int n, coord_t p) {
    
    int x = (int) (p.x * FACTOR) + 1;
    int y = YS - (int) (p.y  * FACTOR) + 1;
    fprintf(js, "    context.font = 'italic 12pt Calibri';\n");
    fprintf(js, "    context.fillText('%d', %d, %d);\n", n+1, x, y);
}

#endif

int compare (const void * a, const void * b)
{
  //  const char **ia = (const char **)a;
   // const char **ib = (const char **)b;
    return ((int) strcmp(*((const char **) a), *((const char **) b)));
}




char isPointInPoly(coord_t * co, coord_t * bldg, short int blen)
{

/*
 # x, y -- x and y coordinates of point
 # a list of tuples [(x, y), (x, y), ...]
 def isPointInPath(x, y, poly):
     num = len(poly)
     i = 0
     j = num - 1
     c = False
     for i in range(num):
     if  ((poly[i][1] > y) != (poly[j][1] > y)) and \
         (x < (poly[j][0] - poly[i][0]) * (y - poly[i][1]) / (poly[j][1] - poly[i][1]) + poly[i][0]):
              c = not c
     j = i
     return c */
    
    short i = 0;
    short j = blen - 1;
    unsigned char c = 0;
    for(i = 0; i < blen; i++) {
        if( ((bldg[i].y > co->y) != (bldg[j].y > co->y)) &&
           (co->x < (bldg[j].x - bldg[i].x) * (co->y - bldg[i].y) / (bldg[j].y - bldg[i].y) + bldg[i].x) )
            c = !c;
        j = i;
    }
    
    return(c);

}

coord_t * findIntersection(coord_t * result, coord_t p1, ray_t * loc1, coord_t p2, ray_t * loc2)
{
    debug_print("Mac %s:\n", loc1->mac);
    debug_print(" -P1:(%lf, %lf), az = %lf, m = %lf, b = %lf\n", p1.x, p1.y, loc1->az, loc1->slope, loc1->offset);
    debug_print(" -P2:(%lf, %lf), az = %lf, m = %lf, b = %lf\n", p2.x, p2.y, loc2->az, loc2->slope, loc2->offset);
    
    double azc1 = loc1->az;
    if(azc1 > 180.0) azc1 -= 180.0;
    double azc2 = loc2->az;
    if(azc2 > 180.0) azc2 -= 180.0;
    debug_print("Az delta = %lf\n", azc1 -azc2);
    // death to parallel
    if(azc1 == azc2) return(NULL);
//    
//    if(loc1->calc) {
//        loc1->slope = tan((MY_PI / 180.0) * (90.0 - loc1->az));
//        loc1->offset = p1.y - (p1.x * loc1->slope);
//        loc2->calc = 0;
//        debug_print("  p1:(%lf, %lf), az = %lf, m = %lf, b = %lf\n", p1.x, p1.y, loc1->az, loc1->slope, loc1->offset);
//    }
//    if(loc2->calc) {
//        loc2->slope = tan((MY_PI / 180.0) * (90.0 - loc2->az));
//        loc2->offset = p2.y - (p2.x * loc2->slope);
//        loc2->calc = 0;
//        debug_print("  p2:(%lf, %lf), az = %lf, m = %lf, b = %lf\n", p2.x, p2.y, loc2->az, loc2->slope, loc2->offset);
//    }
    
    if(loc1->vertical) {
        result->x = p1.x;
        result->y = loc2->slope * p1.x + loc2->offset;
    }
    else if(loc2->vertical) {
        result->x = p2.x;
        result->y = loc1->slope * p2.x + loc1->offset;
    }
    else {
        double deltaSlope = loc2->slope - loc1->slope;
        result->x = (loc1->offset - loc2->offset) / (deltaSlope);
        result->y = ((loc2->slope * loc1->offset) - (loc1->slope * loc2->offset)) / (deltaSlope);
    }
    debug_print("result = (%lf, %lf)\n", result->x, result->y);
#ifdef JSCRIPT
    coord_t r;
//    fprintf(js, "    context.strokeStyle = '#ff0000';\n");
    r.x = result->x - 1;
    r.y = result->y - 1;
    drawpoint(r);
//    drawline(p1, r);
//    drawline(p2, r);
    
    int x = (int) (r.x * FACTOR) + 1;
    int y = YS - (int) (r.y  * FACTOR) + 1;
    char tbuf[200];
    sprintf(tbuf, "(%0.2f, %0.2f)", r.x, r.y );
    fprintf(js, "    context.font = '10pt Calibri';\n");
    fprintf(js, "    context.fillText('%s', %d, %d);\n", tbuf, x, y);

#endif

    
    
//#define PARANOID 1
// this section is not needed if all lines are crossing towards target
#ifdef PARANOID
    if(loc1->az < 180.0) { // going right
        debug_print("1\n");
        if(result->x < p1.x) return(NULL);
    }
    else {                 // going left
        debug_print("2\n");
        if(p1.x < result->x) return(NULL);
    }
    if(loc2->az < 180.0) {  // going right
        debug_print("3\n");
        if(result->x < p2.x) return(NULL);
    }
    else {                  // going left
        debug_print("4\n");
        if(p2.x < result->x) return(NULL);
    }
    
                            // going up
    if((loc1->az < 90.0) || (270.0 < loc1->az)) {
        debug_print("5\n");
        if(result->y < p1.y) return(NULL);
    }
    else {                  // going down
        debug_print("6\n");
        if(p1.y < result->y) return(NULL);
    }
    
                            // going up
    if((loc2->az < 90.0) || (270.0 < loc2->az)) {
        debug_print("7\n");
        if(result->y < p2.y) return(NULL);
    }
    else {                  // going down
        debug_print("8\n");
        if(p2.y < result->y) return(NULL);
    }
    debug_print("no null\n");
#endif
    return(result);
}



int main(int argc, const char * argv[]) {
    debug_print("PI = %lf\n", MY_PI);

#ifdef JSCRIPT
    js = fopen("PLOT.html", "w");
    fprintf(js, "<!DOCTYPE HTML>\n <html>\n  <head>\n   <style>\n    body {\n   margin: 0px;\n");
    fprintf(js, "      padding: 0px;\n    }\n   </style>\n  </head>   \n  <body>\n");
    fprintf(js, "   <canvas id=\"myCanvas\" width=\"%d\" height=\"%d\"></canvas>\n", XS+100, YS+100);
    fprintf(js, "   <script>\n    var canvas = document.getElementById('myCanvas');\n");
    fprintf(js, "    var context = canvas.getContext('2d');\n");
    fprintf(js, "    context.beginPath();\n");
#endif
    
    FILE *file = fopen(argv[1], "r");
    char line[1024];
    char * ptr;
    unsigned char uc;
    unsigned short bcnt = 0;
    unsigned short lcnt = 0;
    unsigned short fcnt = 0;
    
    building_t * blist = NULL;
    location_t * loclist = NULL;
    
    // B001 14.88;8.94 14.88;33.23 25.29;33.23 25.29;15.88 32.23;15.88 32.23;8.94 14.88;8.94
    while(fgets(line, 1024, file)) {
        if(line[0] == '\n') {
            break;
        }
        short int cnt = 0;
        ptr = strtok(line, " ");
        char * bname = strdup((const char *) ptr);
        char * startp = strtok(NULL, " ");
        while(ptr != NULL) {
            ++cnt;
            //            debug_print("%d - %s\n", cnt, ptr);
            //            debug_print("ptr = %p\n", ptr);
            ptr = strtok(NULL, " ");
        }
        unsigned short bnum = bcnt;
        blist = (building_t *) realloc(blist, ((size_t) ++bcnt) * sizeof(building_t));
        blist[bnum].name = bname;
        blist[bnum].cnt = cnt;
        blist[bnum].wificnt = 0;
        blist[bnum].cptr = (coord_t *) realloc(NULL, ((size_t) cnt) * sizeof(coord_t));
        debug_print("Bname = %s, ptr = %p, cnt = %d\n", blist[bnum].name, ptr, blist[bnum].cnt);
        for(uc = 0; uc < cnt; uc++) {
            sscanf(startp, "%lf;%lf", &blist[bnum].cptr[uc].x, &blist[bnum].cptr[uc].y);
            debug_print("%d: %lf, %lf\n",(int) uc, blist[bnum].cptr[uc].x, blist[bnum].cptr[uc].y);
            while(*(++startp) != '\0');
            startp++;
        }
#ifdef JSCRIPT
        ave.x = ave.y = 0.0;
        for(uc = 0; uc < cnt-1; uc++) {
            drawline(blist[bnum].cptr[uc], blist[bnum].cptr[uc+1]);
            ave.x += blist[bnum].cptr[uc].x;
            ave.y += blist[bnum].cptr[uc].y;
        }
        ave.x += blist[bnum].cptr[cnt-1].x;
        ave.y += blist[bnum].cptr[cnt-1].y;
        ave.x = ave.x / ((float) cnt);
        ave.y = ave.y / ((float) cnt);
        sayBuilding(bnum, ave);
#endif

    }
    //            123456789012345678
    // 56.51;5.47 56-4c-18-eb-13-8b;59.3493 88-fe-14-a4-aa-2a;303.0239
    debug_print("%s", "ALL Locations\n");
    while(fgets(line, 1024, file)) {
        if(line[0] == '\n') {
            break;
        }
        short int cnt = 0;
        char * locStr = ptr = strtok(line, " ");
        unsigned short lnum = lcnt;
        
        char * startp = ptr = strtok(NULL, " ");
        while(ptr != NULL) {
            ++cnt;
//            debug_print("parsing cnt = %d - ptr = *%s*\n", cnt, ptr);
            //            debug_print("ptr = %p\n", ptr);
            ptr = strtok(NULL, " ");
        }
        if(cnt > 0) {
            loclist = (location_t *) realloc(loclist, ((size_t) ++lcnt) * sizeof(location_t));
            sscanf(locStr, "%lf;%lf", &loclist[lnum].coord.x, &loclist[lnum].coord.y);
            debug_print("\nLocation %d = (%lf, %lf)\n", lcnt, loclist[lnum].coord.x, loclist[lnum].coord.y);
            loclist[lnum].cnt = cnt;
            loclist[lnum].ray = (ray_t *) realloc(NULL, ((size_t) cnt) * sizeof(ray_t));
            for(uc = 0; uc < cnt; uc++) {
                startp[17] =  '\0';
                memcpy(loclist[lnum].ray[uc].mac, startp, 18);
                loclist[lnum].ray[uc].calc = 1;
                startp += 18;
                sscanf(startp, "%lf", &loclist[lnum].ray[uc].az);
                loclist[lnum].ray[uc].vertical = ((loclist[lnum].ray[uc].az == 0.0) || (loclist[lnum].ray[uc].az == 180.0)) ? 1 : 0;
#ifdef USING_PRECALC
                if((loclist[lnum].ray[uc].az != 0.0) && (loclist[lnum].ray[uc].az != 180.0)) {
                    loclist[lnum].ray[uc].slope = tan((MY_PI / 180.0) * (90.0 - loclist[lnum].ray[uc].az));
                    loclist[lnum].ray[uc].offset = loclist[lnum].coord.y - (loclist[lnum].coord.x * loclist[lnum].ray[uc].slope);
                }
                else {
                    loclist[lnum].ray[uc].slope = 0.0;
                    loclist[lnum].ray[uc].offset = 0.0;
                }
#endif
//                loclist[lnum].ray[uc].slope = 0.0;
//                loclist[lnum].ray[uc].offset = 0.0;
                debug_print("%d, %d: mac = %s, az = %lf, m = %lf, b = %lf \n", (int) lcnt, (int) uc,
                       loclist[lnum].ray[uc].mac, loclist[lnum].ray[uc].az,
                       loclist[lnum].ray[uc].slope, loclist[lnum].ray[uc].offset);
                while(*(++startp) != '\0');
                startp++;
            }
        }
    }
    for(short i = 0; i < lcnt; i++) {
        debug_print("Loc(%d) cnt = %d\n", i, loclist[i].cnt);
    }
    debug_print("%s", "\n\nmatch time\n");
    // loop locations i = (1 to n -1 )
    for(short i = 0; i < lcnt - 1; i++) {
        for(short ii = 0; ii < (short) loclist[i].cnt; ii++) {
        //  inner loop locations j = (i+1, to n)
            for(short j = i + 1; j < lcnt; j++) {
                for(short jj = 0; jj < (short) loclist[j].cnt; jj++) {
                    if(!strcmp(loclist[i].ray[ii].mac, loclist[j].ray[jj].mac)) {
                        debug_print("mac = %s -> Int1(%d,%d), Int2(%d,%d)\n", loclist[i].ray[ii].mac, i, ii, j, jj);
                        coord_t res;
//                        loclist[i].ray[ii].calc = 1;
                        if((loclist[i].ray[ii].calc) && (!loclist[i].ray[ii].vertical)) {
                            debug_print(" *p1:(%lf, %lf), az = %lf, m = %lf, b = %lf\n", loclist[i].coord.x, loclist[i].coord.y,
                                   loclist[i].ray[ii].az, loclist[i].ray[ii].slope, loclist[i].ray[ii].offset);

                            loclist[i].ray[ii].slope = tan((MY_PI / 180.0) * (90.0 - loclist[i].ray[ii].az));
                            loclist[i].ray[ii].offset = loclist[i].coord.y - (loclist[i].coord.x * loclist[i].ray[ii].slope);
                            loclist[i].ray[ii].calc = 0;
                            debug_print("  p1:(%lf, %lf), az = %lf, m = %lf, b = %lf\n", loclist[i].coord.x, loclist[i].coord.y,
                                   loclist[i].ray[ii].az, loclist[i].ray[ii].slope, loclist[i].ray[ii].offset);
                        }
//                        loclist[j].ray[jj].calc = 1;
                        if((loclist[j].ray[jj].calc) && (!loclist[j].ray[jj].vertical)) {
//                            debug_print(" *p2:(%lf, %lf), az = %lf, m = %lf, b = %lf\n", loclist[j].coord.x, loclist[j].coord.y,
//                                   loclist[j].ray[jj].az, loclist[j].ray[jj].slope, loclist[j].ray[jj].offset);
                            loclist[j].ray[jj].slope = tan((MY_PI / 180.0) * (90.0 - loclist[j].ray[jj].az));
                            loclist[j].ray[jj].offset = loclist[j].coord.y - (loclist[j].coord.x * loclist[j].ray[jj].slope);
                            loclist[j].ray[jj].calc = 0;
                            debug_print("  p2:(%lf, %lf), az = %lf, m = %lf, b = %lf\n", loclist[j].coord.x, loclist[j].coord.y,
                                   loclist[j].ray[jj].az, loclist[j].ray[jj].slope, loclist[j].ray[jj].offset);
                        }
                        if(findIntersection(&res, loclist[i].coord, &loclist[i].ray[ii], loclist[j].coord, &loclist[j].ray[jj])) {
                            for(short k = 0; k < bcnt; k++) {
                                debug_print("Bdg = %s, Int1(%d, %d), Int2(%d,%d)\n", blist[k].name, i, ii, j, jj);

                                if(isPointInPoly(&res, blist[k].cptr, blist[k].cnt)) {
                                    if(!blist[k].wificnt) fcnt++;
                                    blist[k].wificnt++;
                                    debug_print("Found %s in %s\n", loclist[i].ray[ii].mac, blist[k].name);
                                }
                            }
                        }
                    }
                }
                
            }
        }
    }
    
    
    // print buildings
    char * * wifibldg = malloc(fcnt * sizeof(char *));
    short j = 0;
    for(short i = 0; i < bcnt; i++) if(blist[i].wificnt) wifibldg[j++] = blist[i].name;
    qsort(wifibldg, fcnt, sizeof(char *), compare);
    for(short i = 0; i < fcnt; i++) puts(wifibldg[i]);
    
    
//    coord_t p1 = {1.0, 2.0};
//    coord_t p2 = {8.0, -1.0};
//    ray_t r1 = {"111", 198.4, 3.000, -1.0000};
//    ray_t r2 = {"222", 251.6, 0.332656, -3.661246};
//    
//    coord_t intersect;
//    if(findIntersection(&intersect, p1, &r1, p2, &r2))
//                debug_print("Intersect = %lf, %lf\n", intersect.x, intersect.y);
#ifdef JSCRIPT
    fprintf(js, "    context.stroke();\n");
    fprintf(js, "   </script>\n  </body>\n </html>\n");
    fclose(js);
#endif
    
    return 0;
}
