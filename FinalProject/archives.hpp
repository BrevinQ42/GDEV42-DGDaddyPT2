/*
Followed this guide: 
https://thomas.trocha.com/blog/entt-de-serialization-with-nlohmann-json/
https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system/465d90e0f5961adc460cd9d1e9358370987fbcd3#snapshot-complete-vs-continuous
*/

#include <iostream>

#include "json.hpp"
#include "entt.hpp"

class NJSONOutputArchive
{
    private:
        nlohmann::json root, current;
    
    public:
        NJSONOutputArchive()
        {
            root = nlohmann::json::array();
        };

        void operator()(std::underlying_type_t<entt::entity> size)
        {
            int a = 0;
            if (!current.empty())
            {
                root.push_back(current);
            }

            current = nlohmann::json::array();
            current.push_back(size);
        }

        void operator()(entt::entity entity)
        {
            current.push_back((uint32_t)entity);
        }

        template<typename T>
        void operator()(entt::entity ent, const T &t)
        {
            current.push_back((uint32_t)ent);

            nlohmann::json json = t;
            current.push_back(json);
        }

        void Close()
        {
            if (!current.empty())
            {
                root.push_back(current);
            }
        }
    
    
        const std::string AsString()
        {
            std::string output = root.dump();
            return output;
        }

};

class NJSONInputArchive
{
    private:
        nlohmann::json root, current;

        int root_index = -1;
        int current_index = 0;

    public:
        NJSONInputArchive(const std::string& json_string)
        {
            root = nlohmann::json::parse(json_string);
        };

        ~NJSONInputArchive(){}

        void next_root()
        {
            root_index++;
            if (root_index >= root.size())
            {
                return;
            }
            current = root[root_index];
            current_index = 0;
        }

        void operator()(std::underlying_type_t<entt::entity> &s)
        {
            next_root();
            int size = current[0].get<int>();
            current_index++;
            s = (std::underlying_type_t<entt::entity>)size;
        }

        void operator()(entt::entity &entity)
        {
            uint32_t e = current[current_index].get<uint32_t>();
            entity = entt::entity(e);
            current_index++;
        }

        template<typename T>
        void operator()(entt::entity &ent, T &t)
        {
            nlohmann::json component_data = current[current_index * 2];

            auto comp = component_data.get<T>();

            t = comp;

            uint32_t _ent = current[current_index * 2 - 1];

            ent =  entt::entity(_ent);

            current_index++;
        }
};