#include "array.h"
#include "lib3d.h"

int main() {
    int w = 1920, h = 1080;
    l3RGB red = {255, 0, 0};
    l3RGB green = {0, 255, 0};
    l3RGB blue = {0, 0, 255};
    l3RGB white = {255, 255, 255};
    {
        l3Texture texture;
        l3Load2DTexture("tex4.ppm", &texture);

        // オブジェクト構築
        l3Object _object;
        l3InitializeObject(&_object);
        l3Vertex* vs[] = {
            l3CreateVertex(5, 5, 5, &red),
            l3CreateVertex(-5, -5, 5, &green),
            l3CreateVertex(5, -5, -5, &blue),
            l3CreateVertex(-5, 5, -5, &white),
        };
        l3SetTransposeObject(&_object, 15, 15, 15);
        l3SetScaleObject(&_object, 1, 1, 1);
        l3Poligon* poligons[] = {
            l3CreatePoligon(vs[0], vs[2], vs[1]),
            l3CreatePoligon(vs[0], vs[3], vs[2]),
            l3CreatePoligon(vs[1], vs[2], vs[3]),
            l3CreatePoligon(vs[0], vs[1], vs[3]),
        };
        poligons[0]->color.r = 255;
        poligons[0]->material = l3PoligonMaterialColor;
        l3Mat32A texture_vertices = {0.5, 0.5, 0, 1, 1, 1};
        l3SetTexturePoligon(poligons[1], &texture, texture_vertices);
        l3SetPoligonsObject(&_object,
                            sizeof(poligons) / sizeof(l3Poligon*), poligons);

        // 変換行列作成
        l3Type camera[4] = {14, 0, -10, 1},
               target[4] = {20, 20, 20, 1},
               upper[4] = {0, 1, 0, 1},
               wc[16] = {0}, cp[16] = {0}, ps[16] = {0}, wcps[16] = {0};
        l3MakeWorldToCameraMat44(camera, target, upper, wc);
        l3MakeCameraToProjectionMat44(120, (double)w / h, 10, 100, cp);
        l3MakeProjectionToScreenMat44(w, h, ps);

        l3Mat44 mats[] = {ps, cp, wc};
        l3MulMat44s44(3, mats, wcps);

        // ラスタマップとバッファーを作る
        l3PixelInfo* map = l3CreateRasterMap(w, h);
        unsigned char* buf = l3CreateBuffer(w, h);

        for (int i = 0; i < 50; i++) {
            // 初期化
            l3ClearVertices(4, vs);
            l3ClearRasterMap(map, w, h);
            l3ClearBuffer(buf, w, h, 255);

            // 動かす
            _object.theta_y = i * 5 * 3.14 / 180.0;

            /* 座標変換のあと、ここにポリゴンを集めて・・・ */
            array* poligons_all = array_new(sizeof(l3Poligon*), true, 10);
            l3AppendPoligonsFromObject(&_object, wcps, w, h, poligons_all);

            /* Z座標でソートして・・・ */
            l3SortPoligonsByMaxZ(poligons_all->length, (l3Poligon**)poligons_all->data);

            /* まとめて描画 */
            array_each_i(poligons_all, l3WriteRasterMap(map, w, h, array_ei));

            array_clear(poligons_all);
            safe_free(poligons_all);

            // 出力
            l3ConvertRasterMapToBuffer(map, buf, w, h);
            char name[20] = {0};
            sprintf(name, "bin/hoge-%03d.ppm", i);
            l3WriteBuffer(buf, w, h, name);
        }

        // 片付け
        safe_free(buf);
        safe_free(map);
        l3DestructVertices(sizeof(vs) / sizeof(l3Vertex*), vs);
        l3DestructPoligons(sizeof(poligons) / sizeof(l3Poligon*), poligons);
    }
}