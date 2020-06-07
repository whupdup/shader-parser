#pragma once

#include <engine/core/common.hpp>
#include <engine/core/string.hpp>

#include <engine/core/array-list.hpp>
#include <engine/core/hash-map.hpp>

class ShaderInfo {
    public:
        struct Variable {
            String typeName;
            String name;
            bool isArray;
            int32 arraySize;
        };

        enum class LayoutType {
            UNIFORM_BUFFER,
            SHADER_STORAGE_BUFFER,
            ATTRIB_IN,
            ATTRIB_OUT,
            UNIFORM,

            INVALID
        };

        struct Layout {
            LayoutType type = LayoutType::INVALID;

            HashMap<String, int32> options; // TODO: confirm all option values are numeric or nonexistent
            ArrayList<String> memoryQualifiers;
            String name;
            String typeQualifier;

            ArrayList<ShaderInfo::Variable> body;
        };

        static const char* stringifyLayoutType(enum LayoutType type);

        ShaderInfo() = default;

        bool parse(std::istream& shaderData);

        ArrayList<Layout>& getLayoutInfo();
        const ArrayList<Layout>& getLayoutInfo() const;
    private:
        NULL_COPY_AND_ASSIGN(ShaderInfo);

        ArrayList<Layout> layoutInfo;
};
