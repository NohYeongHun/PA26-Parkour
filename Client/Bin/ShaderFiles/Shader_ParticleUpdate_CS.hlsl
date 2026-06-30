// --- Resources (CModel::Ready_GPU_Buffers에서 생성한 버퍼들) ---
#define THREAD_X 64
#define THREAD_Y 1
#define THREAD_Z 1

struct ParticleState
{
    float4 Right;
    float4 Up;          
    float4 Look;       
    float4 Pos;
    
    float2 LifeTime;
    float2 Delay;
    
    float4 VelTail;         //x,y,z = Vel / w = Tail
    
    float Phase;
    float3 _pad0;
};

struct ParticleStatic
{
    float4 DefaultPos;
    float Speed; 
    float Delay;
    float2 _pad0; 
};


//상수버퍼
cbuffer OptionCB : register(b0)
{                                       
    float3 Pivot;  // 16B(= float3 + float)}      16으로 맞춰줘야함 만약 float3 , 다음 float3가 또오면 float3 fTest; float _pad0; 이런식으로 맞춰줘야한다는거 같음.
    uint IsLoop;
    
    uint IsStretch;
    uint IsSprite;
    uint IsDelay;
    float _pad0;
}

cbuffer SpeedCB : register(b1)
{
    float DeltaTime;
    float SpreadWeight;
    float DropWeight;
    float RotationWeight;
    
    float Gravity;
    float fStretchWeight;
    float2 fStretchRange;
    
    float fSpriteWeight;
    float fSpriteDefault;
    float _pad[2];
}

//SRV
StructuredBuffer<ParticleStatic> g_ParticleStatic : register(t0);

//UAV 구조체로 정보입력
RWStructuredBuffer<ParticleState> g_ParticleState : register(u0);

float4 Spread(float4 Pos, float Speed)
{
    float4 MoveDir = float4(normalize(Pos.xyz - Pivot), 0.f);
    
    float Speedw = Speed * SpreadWeight;
  
    float4 Position = Pos + MoveDir * Speedw * DeltaTime;
    
    return Position;
}

float4 Drop(float4 Pos, float Speed)
{
    float4 MoveDir = float4(0.f, -1.f, 0.f, 0.f);
    
    float Speedw = Speed * DropWeight;
    
    float4 Position = Pos + MoveDir * Speedw * DeltaTime;
    
    return Position;
}

float4 Rotation(float4 Pos, float Speed)
{
    float4 LocalPos = Pos - float4(Pivot.xyz, 1.f);
    
    float Speedw = Speed * RotationWeight;
    
    float fAngle = Speedw * DeltaTime;
    
    float sin = 0.f;
    float cos = 0.f;
    
    sincos(fAngle, sin, cos);
    
    float PosX = (cos * LocalPos.x) - (sin * LocalPos.z);
    float PosZ = (sin * LocalPos.x) + (cos * LocalPos.z);
    
    LocalPos.x = PosX;
    LocalPos.z = PosZ;
    
    float4 Position = LocalPos + float4(Pivot.xyz, 1.f);
  
    return Position;
}


[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    
    if (IsDelay == 1)
    {
        if (g_ParticleStatic[i].Delay >= g_ParticleState[i].Delay.x)
        {
            g_ParticleState[i].Delay.x += DeltaTime;
            g_ParticleState[i].Delay.y = 1.f;
            return;
        }
        else
        {
            g_ParticleState[i].Delay.y = 0.f;
        }
    }
    
    
    //if (g_ParticleState[i].LifeTime.x <= g_ParticleState[i].LifeTime.y)
    //{
    float4 PreviousPos = g_ParticleState[i].Pos;
        
    
    float4 Position = g_ParticleState[i].Pos;
    float Speed = g_ParticleStatic[i].Speed;
   
    if (SpreadWeight > 0)
    {
        Position = Spread(Position, Speed);
    }

    if (DropWeight > 0)
    {
        Position = Drop(Position, Speed);
    }
   
    if (RotationWeight > 0)
    {
        Position = Rotation(Position, Speed);
    }
    
    //여러 동작 처리들 다해서 나온 포지션 값 대입
    g_ParticleState[i].Pos = Position;
    
    g_ParticleState[i].LifeTime.x += DeltaTime;
    
    
     // Stretch 빌보드 활성화
   if(IsStretch == 1)
    {
        float4 Velocity = (g_ParticleState[i].Pos - PreviousPos) / DeltaTime;

        g_ParticleState[i].VelTail = Velocity;
        
        float fSpeed = length(Velocity);
        
        float tailLen = clamp(fSpeed * fStretchWeight, fStretchRange.x, fStretchRange.y);
        
        g_ParticleState[i].VelTail.w = tailLen;
    }
    
    // Sprite 화성화
    if(IsSprite == 1)
    {
        //
        float4 Velocity = (g_ParticleState[i].Pos - PreviousPos) / DeltaTime;

        float fSpeed = length(Velocity);
        
        float fPhase = g_ParticleState[i].Phase;     
        
        fPhase += (fSpriteDefault + (fSpriteWeight * fSpeed)) * DeltaTime;
        
        g_ParticleState[i].Phase = fPhase;
    }
    
    //
    if(IsLoop == 1)
    {
        if(g_ParticleState[i].LifeTime.x >= g_ParticleState[i].LifeTime.y)
        {
            g_ParticleState[i].LifeTime.x = 0.f;
            g_ParticleState[i].Pos = g_ParticleStatic[i].DefaultPos;
            g_ParticleState[i].VelTail = float4(0.f, 0.f, 0.f, 0.f);
            g_ParticleState[i].Delay.x = 0.f; 
            g_ParticleState[i].Delay.y = 1.f;

        }
    }
    else
    {
        if (g_ParticleState[i].LifeTime.x >= g_ParticleState[i].LifeTime.y)
        {
            g_ParticleState[i].Pos.y -= Gravity * DeltaTime;
        }
    }
}


