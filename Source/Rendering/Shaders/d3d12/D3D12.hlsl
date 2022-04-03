// This is the most common case of a vertex + pixel shader combination. If tessellation/geometry is needed,
// there is a different root signature for that
#define RootSignatureGraphics \
	"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_HULL_SHADER_ROOT_ACCESS | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS),"\
	"DescriptorTable(CBV(b0, numDescriptors = unbounded), visibility = SHADER_VISIBILITY_VERTEX),"\
	"DescriptorTable(SRV(t0, numDescriptors = unbounded), visibility = SHADER_VISIBILITY_VERTEX),"\
	"DescriptorTable(UAV(u0, numDescriptors = unbounded), visibility = SHADER_VISIBILITY_VERTEX),"\
	"DescriptorTable(Sampler(s0, numDescriptors = unbounded), visibility = SHADER_VISIBILITY_VERTEX),"\
	"DescriptorTable(CBV(b0, numDescriptors = unbounded), visibility = SHADER_VISIBILITY_PIXEL),"\
	"DescriptorTable(SRV(t0, numDescriptors = unbounded), visibility = SHADER_VISIBILITY_PIXEL),"\
	"DescriptorTable(UAV(u0, numDescriptors = unbounded), visibility = SHADER_VISIBILITY_PIXEL),"\
	"DescriptorTable(Sampler(s0, numDescriptors = unbounded), visibility = SHADER_VISIBILITY_PIXEL)"
	
// The compute queue always uses D3D12_SHADER_VISIBILITY_ALL because it has only one active stage.
#define RootSignatureCompute "RootFlags(0),"\
	"DescriptorTable(CBV(b0, numDescriptors = unbounded), visibility = SHADER_VISIBILITY_ALL),"\
	"DescriptorTable(SRV(t0, numDescriptors = unbounded), visibility = SHADER_VISIBILITY_ALL),"\
	"DescriptorTable(UAV(u0, numDescriptors = unbounded), visibility = SHADER_VISIBILITY_ALL),"\
	"DescriptorTable(Sampler(s0, numDescriptors = unbounded), visibility = SHADER_VISIBILITY_ALL),"
