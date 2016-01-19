/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "material.h"
#include "material_manager.h"
#include "material_resource.h"
#include "memory.h"
#include "resource_manager.h"
#include "shader.h"
#include "shader_manager.h"
#include "texture_resource.h"
#include <bgfx/bgfx.h>
#include <string.h> // memcpy

namespace crown
{

using namespace material_resource;

void Material::create(const MaterialResource* mr)
{
	const uint32_t size = mr->dynamic_data_size;
	const uint32_t offt = mr->dynamic_data_offset;
	char* base = (char*)mr + offt;
	data = (char*)default_allocator().allocate(size);
	memcpy(data, base, size);
	resource = mr;
}

void Material::destroy() const
{
	default_allocator().deallocate(data);
}

void Material::bind(ResourceManager& rm, ShaderManager& sm) const
{
	// Set samplers
	for (uint32_t i = 0; i < resource->num_textures; ++i)
	{
		TextureData* td   = get_texture_data(resource, i);
		TextureHandle* th = get_texture_handle(resource, i, data);

		bgfx::UniformHandle sampler;
		bgfx::TextureHandle texture;
		sampler.idx = th->sampler_handle;

		const TextureResource* teximg = (TextureResource*)rm.get(TEXTURE_TYPE, td->id);
		texture.idx = teximg->handle.idx;

		bgfx::setTexture(i, sampler, texture);
	}

	// Set uniforms
	for (uint32_t i = 0; i < resource->num_uniforms; ++i)
	{
		UniformHandle* uh = get_uniform_handle(resource, i, data);

		bgfx::UniformHandle buh;
		buh.idx = uh->uniform_handle;
		bgfx::setUniform(buh, (char*)uh + sizeof(uh->uniform_handle));
	}

	const ShaderManager::ShaderData& sd = sm.get(resource->shader);
	bgfx::setState(sd.state);
	bgfx::submit(0, sd.program);
}

void Material::set_float(const char* name, float val)
{
	char* p = (char*)get_uniform_handle_by_string(resource, name, data);
	*((float*)(p + sizeof(uint32_t))) = val;
}

void Material::set_vector2(const char* name, const Vector2& val)
{
	char* p = (char*)get_uniform_handle_by_string(resource, name, data);
	*((Vector2*)(p + sizeof(uint32_t))) = val;
}

void Material::set_vector3(const char* name, const Vector3& val)
{
	char* p = (char*)get_uniform_handle_by_string(resource, name, data);
	*((Vector3*)(p + sizeof(uint32_t))) = val;
}

} // namespace crown
