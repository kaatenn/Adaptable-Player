//
// Created by 86137 on 2023/12/30.
//

#ifndef ADAPTABLE_SERVER_APPLICATIONPROTOCOLBASE_H
#define ADAPTABLE_SERVER_APPLICATIONPROTOCOLBASE_H

#include "vector"
#include "string"

namespace kaatenn {
    class ApplicationProtocolBase {
    public:
        /**
         * @brief Converts the current object's state to a string for transmission.
         *
         * This method should serialize the internal state of the derived class into
         * a string format suitable for network transmission.
         *
         * @return A string representation of the object's state.
         */
        [[nodiscard]] virtual std::string serialize() const = 0;


        /**
         * @brief Manages and processes incoming data packages.
         *
         * Stores the received data in the receive buffer and handles the end-of-data
         * marking and processing. This method should be overridden to define custom
         * behavior for managing incoming data.
         *
         * @param segment A pointer to the data segment to be processed.
         *
         * @return True if the data segment was successfully processed and could return a response.
         */
        virtual bool process_segment(char* segment, int length) = 0;

        /**
         * @brief Handles errors encountered by the protocol.
         *
         * This method should be implemented to define how the derived class handles
         * various errors that may occur during the protocol's operation.
         */
        virtual void error_handler() = 0;

        /**
         * @brief Returns the response to be sent to the client.
         *
         * This method should be implemented to define how the derived class returns
         * the response to be sent to the client.
         * @return The response to be sent to the client.
         */
        virtual std::string get_response() = 0;

        virtual std::string get_params() const = 0;

        virtual void reset() = 0;

        // Rule of five: allowing move operations while preventing copy operations.
        ApplicationProtocolBase() = default;

        ApplicationProtocolBase(ApplicationProtocolBase &&) noexcept = default;

        ApplicationProtocolBase &operator=(ApplicationProtocolBase &&) noexcept = default;

        ApplicationProtocolBase(const ApplicationProtocolBase &) = delete;

        ApplicationProtocolBase &operator=(const ApplicationProtocolBase &) = delete;
    protected:
        std::string response;
    };

}


#endif //ADAPTABLE_SERVER_APPLICATIONPROTOCOLBASE_H
