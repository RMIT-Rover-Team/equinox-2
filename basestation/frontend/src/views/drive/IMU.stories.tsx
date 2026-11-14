import type { Meta, StoryObj } from "@storybook/react-vite";
import IMU, { Horizon } from "./IMU";

const meta = {
  title: "Drive/Horizon",
  component: Horizon,
  args: {
    roll: 0,
    pitch: 0,
    yaw: 0,
  },
  argTypes: {
    roll: {
      description:
        "Degrees: negative tilts the horizon left; positive tilts it right.",
      control: { type: "range", min: -180, max: 180, step: 0.1 },
    },
    pitch: { control: { type: "range", min: -90, max: 90, step: 0.1 } },
    yaw: { control: { type: "range", min: -180, max: 180, step: 0.1 } },
  },
  parameters: {
    layout: "centered",
  },
  decorators: [
    (Story) => (
      <div style={{ width: 400 }}>
        <Story />
      </div>
    ),
  ],
} satisfies Meta<typeof Horizon>;

export default meta;
type Story = StoryObj<typeof meta>;

export const Interactive: Story = {};

export const Animated: Story = {
  render: () => <IMU />,
  parameters: { controls: { disable: true } },
};
