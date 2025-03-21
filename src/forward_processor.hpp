#ifndef FORWARDING_PROCESSOR_HPP
#define FORWARDING_PROCESSOR_HPP

#include "processor.hpp"

class ForwardingProcessor : public Processor {
private:
    // Forwarding unit
    ForwardingUnit forwarding_unit;
    
    // Hazard detection
    HazardDetectionUnit hazard_unit;
    
    // Pipeline diagram specific to forwarding
    void update_pipeline_diagram() override;
    
public:
    ForwardingProcessor() = default;
    
    // Override specific pipeline stages
    void decode() override;
    void execute() override;
};

#endif // FORWARDING_PROCESSOR_HPP