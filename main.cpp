#include <windows.h>
#include <vector>
#include <cmath>
#include <string>

struct Vec2 { float x,y; };
struct Wall { RECT r; };
struct Player { Vec2 p; bool visible; };

static Vec2 sub(Vec2 a, Vec2 b){ return {a.x-b.x,a.y-b.y}; }
static float cross(Vec2 a, Vec2 b){ return a.x*b.y-a.y*b.x; }

static bool segHit(Vec2 a,Vec2 b,Vec2 c,Vec2 d){
    Vec2 r=sub(b,a), s=sub(d,c);
    float den=cross(r,s);
    if(std::fabs(den)<0.0001f) return false;
    float t=cross(sub(c,a),s)/den;
    float u=cross(sub(c,a),r)/den;
    return t>=0 && t<=1 && u>=0 && u<=1;
}

static bool blocked(Vec2 a,Vec2 b,const Wall&w){
    if(a.x>=w.r.left&&a.x<=w.r.right&&a.y>=w.r.top&&a.y<=w.r.bottom) return true;
    Vec2 p1={(float)w.r.left,(float)w.r.top};
    Vec2 p2={(float)w.r.right,(float)w.r.top};
    Vec2 p3={(float)w.r.right,(float)w.r.bottom};
    Vec2 p4={(float)w.r.left,(float)w.r.bottom};
    return segHit(a,b,p1,p2)||segHit(a,b,p2,p3)||segHit(a,b,p3,p4)||segHit(a,b,p4,p1);
}

static bool visible(Vec2 cam,Vec2 p,const std::vector<Wall>&walls){
    for(auto&w:walls) if(blocked(cam,p,w)) return false;
    return true;
}

static bool menuOpen=false;
static bool espEnabled=true;
static bool showLines=true;
static bool showOnlyVisible=true;
static int maxDistance=100;

static Vec2 camera{100,300};
static std::vector<Wall> walls={
    {{300,120,390,520}},{{570,40,650,360}},{{760,430,900,540}}
};
static std::vector<Player> players={
    {{180,150},false},{{470,180},false},{{500,480},false},
    {{700,180},false},{{850,260},false},{{1030,480},false}
};

static void drawText(HDC dc,int x,int y,const wchar_t*s,COLORREF c=RGB(235,235,240)){
    SetBkMode(dc,TRANSPARENT); SetTextColor(dc,c);
    TextOutW(dc,x,y,s,lstrlenW(s));
}

static void button(HDC dc,RECT r,const wchar_t*text,bool on){
    HBRUSH b=CreateSolidBrush(on?RGB(50,145,80):RGB(65,65,72));
    FillRect(dc,&r,b); DeleteObject(b);
    FrameRect(dc,&r,(HBRUSH)GetStockObject(WHITE_BRUSH));
    drawText(dc,r.left+12,r.top+7,text);
}

static LRESULT CALLBACK WndProc(HWND h,UINT m,WPARAM wp,LPARAM lp){
    if(m==WM_TIMER){
        static float t=0; t+=0.02f;
        players[0].p.y=150+std::sin(t)*60;
        players[3].p.y=180+std::cos(t*.7f)*90;
        for(auto&p:players) p.visible=visible(camera,p.p,walls);
        InvalidateRect(h,nullptr,FALSE);
        return 0;
    }

    if(m==WM_KEYDOWN && wp==VK_INSERT){
        menuOpen=!menuOpen;
        InvalidateRect(h,nullptr,FALSE);
        return 0;
    }

    if(m==WM_LBUTTONDOWN && menuOpen){
        int x=LOWORD(lp), y=HIWORD(lp);
        RECT r1={760,90,1020,130};
        RECT r2={760,140,1020,180};
        RECT r3={760,190,1020,230};
        RECT r4={760,240,1020,280};
        if(PtInRect(&r1,{x,y})) espEnabled=!espEnabled;
        if(PtInRect(&r2,{x,y})) showLines=!showLines;
        if(PtInRect(&r3,{x,y})) showOnlyVisible=!showOnlyVisible;
        if(PtInRect(&r4,{x,y})) maxDistance=(maxDistance==100?250:100);
        InvalidateRect(h,nullptr,FALSE);
        return 0;
    }

    if(m==WM_PAINT){
        PAINTSTRUCT ps; HDC dc=BeginPaint(h,&ps);
        RECT client; GetClientRect(h,&client);
        HBRUSH bg=CreateSolidBrush(RGB(20,20,24));
        FillRect(dc,&client,bg); DeleteObject(bg);

        HBRUSH wb=CreateSolidBrush(RGB(90,90,100));
        for(auto&w:walls) FillRect(dc,&w.r,wb);
        DeleteObject(wb);

        HBRUSH cb=CreateSolidBrush(RGB(70,160,255));
        Ellipse(dc,(int)camera.x-7,(int)camera.y-7,(int)camera.x+7,(int)camera.y+7);
        DeleteObject(cb);

        if(espEnabled){
            for(auto&p:players){
                if(showOnlyVisible && !p.visible) continue;
                HPEN pen=CreatePen(PS_SOLID,3,p.visible?RGB(70,255,110):RGB(130,130,135));
                HBRUSH br=CreateSolidBrush(p.visible?RGB(40,170,75):RGB(90,90,95));
                auto op=SelectObject(dc,pen); auto ob=SelectObject(dc,br);
                Rectangle(dc,(int)p.p.x-10,(int)p.p.y-18,(int)p.p.x+10,(int)p.p.y+18);
                SelectObject(dc,op); SelectObject(dc,ob);
                DeleteObject(pen); DeleteObject(br);
                if(showLines && p.visible){
                    MoveToEx(dc,(int)camera.x,(int)camera.y,nullptr);
                    LineTo(dc,(int)p.p.x,(int)p.p.y);
                }
            }
        }

        drawText(dc,20,20,L"LOS ESP LAB — standalone test scene");
        drawText(dc,20,45,L"INSERT = open/close settings");

        if(menuOpen){
            RECT panel={730,55,1050,330};
            HBRUSH pb=CreateSolidBrush(RGB(32,32,38));
            FillRect(dc,&panel,pb); DeleteObject(pb);
            FrameRect(dc,&panel,(HBRUSH)GetStockObject(WHITE_BRUSH));
            drawText(dc,760,65,L"ESP SETTINGS");
            button(dc,{760,90,1020,130},L"ESP",espEnabled);
            button(dc,{760,140,1020,180},L"Lines",showLines);
            button(dc,{760,190,1020,230},L"Only visible",showOnlyVisible);
            button(dc,{760,240,1020,280},maxDistance==100?L"Distance: 100":L"Distance: 250",true);
            drawText(dc,760,300,L"Click buttons to change settings");
        }

        EndPaint(h,&ps); return 0;
    }

    if(m==WM_DESTROY){ PostQuitMessage(0); return 0; }
    return DefWindowProcW(h,m,wp,lp);
}

int WINAPI WinMain(HINSTANCE inst,HINSTANCE,LPSTR,int show){
    WNDCLASSW wc{};
    wc.lpfnWndProc=WndProc;
    wc.hInstance=inst;
    wc.lpszClassName=L"RustLOSLabV2";
    wc.hCursor=LoadCursor(nullptr,IDC_ARROW);
    RegisterClassW(&wc);

    HWND h=CreateWindowExW(
        0,wc.lpszClassName,L"Rust LOS ESP Lab V2",
        WS_OVERLAPPEDWINDOW,100,100,1120,640,
        nullptr,nullptr,inst,nullptr
    );
    ShowWindow(h,show);
    SetTimer(h,1,16,nullptr);

    MSG msg{};
    while(GetMessageW(&msg,nullptr,0,0)>0){
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}
